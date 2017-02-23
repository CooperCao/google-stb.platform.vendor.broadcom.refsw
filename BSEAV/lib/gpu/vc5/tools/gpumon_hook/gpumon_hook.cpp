/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  GPUMonitor intercept hook

FILE DESCRIPTION
Provides entry points to match the VC5 driver
=============================================================================*/

// The GL/EGL headers are included as part of this
#include "api.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include <setjmp.h>
#include <ctype.h>
#include <assert.h>

#include <sys/types.h>

#include "platform.h"
#include <map>
#include <set>
#include <string>
#include <array>

#include "packet.h"
#include "remote.h"
#include "control.h"
#include "archive.h"

#include "circularbuffer.h"

#ifndef WIN32
#include <dlfcn.h>
#endif

#ifdef ANDROID
#include <cutils/log.h>
#else
#define ALOGD printf
#define ALOGE printf
#endif

// The version of the SpyHook API (SpyHook<->SpyTool)
// Update this when the interface between the SpyHook & SpyTool changes
#define SPYHOOK_MAJOR_VER 2
#define SPYHOOK_MINOR_VER 2

// The version of the binary capture format
// Update this when the capture data changes
#define CAPTURE_MAJOR_VER 1
#define CAPTURE_MINOR_VER 5

#if KHRN_GLES31_DRIVER
const unsigned int GLESVER = 31;
#else
const unsigned int GLESVER = 3;
#endif

// Set the XML buffer size to a reasonable amount
#define XML_BUFSIZE     ( 5 * 1024 )

#define FOURCC(a,b,c,d)		(((a) << 24) | ((b) << 16) | ((c) << 8) | (d))

#define DLLEXPORTENTRY

static Packet  *sCurPacket = NULL;
static bool     sEnabled = true;
static bool     sOrphaned = false;
static bool     sSendNextRet = true;
static uint32_t sBottleneckMode = Control::eUnset;
static uint32_t sPerfNumFrames = 1;
static float    sPerfNumSeconds = 0.0f;
static uint32_t sFrameCnt = 0;
static uint32_t sTotalFrameCnt = 0;
static uint32_t sTimeStampMs = 0;
static bool     sCaptureEvents = false;
static int64_t  sEventTimebaseOffset = 0;
static GLint    sEventApiTrack = -1;
static GLint    sApiEventCode = -1;
static GLint    sTextureApiEventCode = -1;
static uint32_t sEventId = 0;

static bool     sCaptureStream = false;
static Archive *sCaptureArchive = NULL;
static bool     sIgnoreEGLTerminate = false;

static EGLBoolean            sEventOverflow = EGL_FALSE;
static std::set<eGLCommand>  sMinimalModeCmds;

static std::map< uint32_t, std::vector<uint32_t> > sVarset;

static REAL_GL_API_TABLE sRealFuncs;

const char               *gLastFuncName = 0;
bool                      gAPIReentrancy = false;

#define Real(f) sRealFuncs.real_##f

static PFNEGLGETPERFCOUNTERCONSTANTBRCMPROC    sGetPerfCounterConstantBRCM;
static PFNEGLGETPERFCOUNTERGROUPINFOBRCMPROC   sGetPerfCounterGroupInfoBRCM;
static PFNEGLGETPERFCOUNTERINFOBRCMPROC        sGetPerfCounterInfoBRCM;
static PFNEGLSETPERFCOUNTINGBRCMPROC           sSetPerfCountingBRCM;
static PFNEGLCHOOSEPERFCOUNTERSBRCMPROC        sChoosePerfCountersBRCM;
static PFNEGLGETPERFCOUNTERDATABRCMPROC        sGetPerfCounterDataBRCM;

static PFNEGLGETEVENTCONSTANTBRCMPROC          sGetEventConstantBRCM;
static PFNEGLGETEVENTTRACKINFOBRCMPROC         sGetEventTrackInfoBRCM;
static PFNEGLGETEVENTINFOBRCMPROC              sGetEventInfoBRCM;
static PFNEGLGETEVENTDATAFIELDINFOBRCMPROC     sGetEventDataFieldInfoBRCM;
static PFNEGLSETEVENTCOLLECTIONBRCMPROC        sSetEventCollectionBRCM;
static PFNEGLGETEVENTDATABRCMPROC              sGetEventDataBRCM;

extern "C"
{
   static void gpumon_initialize();
   static void send_thread_change(uint32_t threadID, EGLContext context);
   static void SendPerfDataPacket();
}

//////////////////////////////////////////////////////////////////////////////////////

#define EVENT_BUFFER_CHUNK        (2 * 1024* 1024)              /* bytes */
#define MAX_EVENT_BUFFER_SIZE     (EVENT_BUFFER_CHUNK * 10)   /* bytes */
#define BLANK_EVENTS_BUFFER_SIZE    28 * 2                        /* 2 small API event */

CircularEventBuffer sEventBuffer(MAX_EVENT_BUFFER_SIZE, EVENT_BUFFER_CHUNK);

// Used to track the length of time between the first API
// event and the first kept event in the circular buffer
uint64_t sFirstApiCapTime;

// This will contain two blank events (no_command): start and end
uint8_t sBlankEventBuffer[BLANK_EVENTS_BUFFER_SIZE];

//////////////////////////////////////////////////////////////////////////////////////

class APIInitAndLock
{
public:
   APIInitAndLock(const char *funcName, bool detectThreadChange = true)
   {
      bool threadChanged = false;

      m_needsUnlock = plGlobalLock(funcName, &threadChanged);
      gpumon_initialize();

      gLastFuncName = funcName;

      if (detectThreadChange && threadChanged)
         send_thread_change(plGetThreadID(), Real(eglGetCurrentContext()));

      if (!gAPIReentrancy && !sOrphaned && sBottleneckMode != Control::eUnset)
      {
         bool sendMe = false;

         if (sPerfNumFrames > 0 && sFrameCnt >= sPerfNumFrames)
         {
            sendMe = true;
         }
         else if (sPerfNumSeconds > 0.0f)
         {
            uint32_t nowMs = plGetTimeNowMs();
            if (nowMs - sTimeStampMs > (uint32_t)(1000.0f * sPerfNumSeconds))
               sendMe = true;
         }

         if (sendMe)
            SendPerfDataPacket();
      }
   }

   ~APIInitAndLock()
   {
      if (m_needsUnlock)
         plGlobalUnlock();
   }

private:
   bool m_needsUnlock;
};

///////////////////////////////////////////////////////////////////////////////////////////

class ErrorGL
{
private:
   GLenum m_error;

public:
   ErrorGL() : m_error(GL_NO_ERROR) {}
   ErrorGL(GLenum err) : m_error(err) {}

   void Set(GLenum err) { m_error = err; }
   GLenum Get() const { return m_error; }
};

class ErrorEGL
{
private:
   EGLint m_error;

public:
   ErrorEGL() : m_error(EGL_SUCCESS) {}
   ErrorEGL(EGLint err) : m_error(err) {}

   void Set(EGLint err) { m_error = err; }
   EGLint Get() const { return m_error; }
};

// Per thread maps for remembering errors
static std::map< uint32_t, ErrorGL>  sGLErrors;
static std::map< uint32_t, ErrorEGL> sEGLErrors;

///////////////////////////////////////////////////////////////////////////////////////////

class Buffer
{
public:
   Buffer() : m_maxByteIndx(0), m_maxShortIndx(0), m_maxIntIndx(0),
              m_nextHighestByteIndx(0), m_nextHighestShortIndx(0), m_nextHighestIntIndx(0)
   {}

   GLuint m_maxByteIndx;
   GLuint m_maxShortIndx;
   GLuint m_maxIntIndx;

   GLuint m_nextHighestByteIndx;
   GLuint m_nextHighestShortIndx;
   GLuint m_nextHighestIntIndx;
};

class Context
{
public:
   std::map<GLuint, Buffer>   m_buffers;
};

std::map<GLuint, Context>  sContexts;

static void Packetize()
{
}

static void InitExtensions()
{
   sGetPerfCounterConstantBRCM = (PFNEGLGETPERFCOUNTERCONSTANTBRCMPROC)Real(eglGetProcAddress)("eglGetPerfCounterConstantBRCM");
   sGetPerfCounterGroupInfoBRCM = (PFNEGLGETPERFCOUNTERGROUPINFOBRCMPROC)Real(eglGetProcAddress)("eglGetPerfCounterGroupInfoBRCM");
   sGetPerfCounterInfoBRCM = (PFNEGLGETPERFCOUNTERINFOBRCMPROC)Real(eglGetProcAddress)("eglGetPerfCounterInfoBRCM");
   sSetPerfCountingBRCM = (PFNEGLSETPERFCOUNTINGBRCMPROC)Real(eglGetProcAddress)("eglSetPerfCountingBRCM");
   sChoosePerfCountersBRCM = (PFNEGLCHOOSEPERFCOUNTERSBRCMPROC)Real(eglGetProcAddress)("eglChoosePerfCountersBRCM");
   sGetPerfCounterDataBRCM = (PFNEGLGETPERFCOUNTERDATABRCMPROC)Real(eglGetProcAddress)("eglGetPerfCounterDataBRCM");
   sGetEventConstantBRCM = (PFNEGLGETEVENTCONSTANTBRCMPROC)Real(eglGetProcAddress)("eglGetEventConstantBRCM");
   sGetEventTrackInfoBRCM = (PFNEGLGETEVENTTRACKINFOBRCMPROC)Real(eglGetProcAddress)("eglGetEventTrackInfoBRCM");
   sGetEventInfoBRCM = (PFNEGLGETEVENTINFOBRCMPROC)Real(eglGetProcAddress)("eglGetEventInfoBRCM");
   sGetEventDataFieldInfoBRCM = (PFNEGLGETEVENTDATAFIELDINFOBRCMPROC)Real(eglGetProcAddress)("eglGetEventDataFieldInfoBRCM");
   sSetEventCollectionBRCM = (PFNEGLSETEVENTCOLLECTIONBRCMPROC)Real(eglGetProcAddress)("eglSetEventCollectionBRCM");
   sGetEventDataBRCM = (PFNEGLGETEVENTDATABRCMPROC)Real(eglGetProcAddress)("eglGetEventDataBRCM");

   if (!sGetPerfCounterConstantBRCM || !sGetPerfCounterGroupInfoBRCM || !sGetPerfCounterInfoBRCM ||
       !sSetPerfCountingBRCM || !sChoosePerfCountersBRCM || !sGetPerfCounterDataBRCM ||
       !sGetEventConstantBRCM || !sGetEventTrackInfoBRCM || !sGetEventInfoBRCM || !sGetEventDataFieldInfoBRCM ||
       !sSetEventCollectionBRCM || !sGetEventDataBRCM)
       ALOGE("**** Performance / event extensions not found\n");
}

template <typename T1>
void Packetize(T1 u)
{
   sCurPacket->AddItem(u);
}

template <typename T1, typename T2>
void Packetize(T1 t1, T2 t2)
{
   sCurPacket->AddItem(t1);
   sCurPacket->AddItem(t2);
}

template <typename T1, typename T2, typename T3>
void Packetize(T1 t1, T2 t2, T3 t3)
{
   sCurPacket->AddItem(t1);
   sCurPacket->AddItem(t2);
   sCurPacket->AddItem(t3);
}

template <typename T1, typename T2, typename T3, typename T4>
void Packetize(T1 t1, T2 t2, T3 t3, T4 t4)
{
   sCurPacket->AddItem(t1);
   sCurPacket->AddItem(t2);
   sCurPacket->AddItem(t3);
   sCurPacket->AddItem(t4);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5>
void Packetize(T1 t1, T2 t2, T3 t3, T4 t4, T5 t5)
{
   sCurPacket->AddItem(t1);
   sCurPacket->AddItem(t2);
   sCurPacket->AddItem(t3);
   sCurPacket->AddItem(t4);
   sCurPacket->AddItem(t5);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
void Packetize(T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6)
{
   sCurPacket->AddItem(t1);
   sCurPacket->AddItem(t2);
   sCurPacket->AddItem(t3);
   sCurPacket->AddItem(t4);
   sCurPacket->AddItem(t5);
   sCurPacket->AddItem(t6);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
void Packetize(T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7)
{
   sCurPacket->AddItem(t1);
   sCurPacket->AddItem(t2);
   sCurPacket->AddItem(t3);
   sCurPacket->AddItem(t4);
   sCurPacket->AddItem(t5);
   sCurPacket->AddItem(t6);
   sCurPacket->AddItem(t7);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
void Packetize(T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8)
{
   sCurPacket->AddItem(t1);
   sCurPacket->AddItem(t2);
   sCurPacket->AddItem(t3);
   sCurPacket->AddItem(t4);
   sCurPacket->AddItem(t5);
   sCurPacket->AddItem(t6);
   sCurPacket->AddItem(t7);
   sCurPacket->AddItem(t8);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
void Packetize(T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9)
{
   sCurPacket->AddItem(t1);
   sCurPacket->AddItem(t2);
   sCurPacket->AddItem(t3);
   sCurPacket->AddItem(t4);
   sCurPacket->AddItem(t5);
   sCurPacket->AddItem(t6);
   sCurPacket->AddItem(t7);
   sCurPacket->AddItem(t8);
   sCurPacket->AddItem(t9);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10>
void Packetize(T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9, T10 t10)
{
   sCurPacket->AddItem(t1);
   sCurPacket->AddItem(t2);
   sCurPacket->AddItem(t3);
   sCurPacket->AddItem(t4);
   sCurPacket->AddItem(t5);
   sCurPacket->AddItem(t6);
   sCurPacket->AddItem(t7);
   sCurPacket->AddItem(t8);
   sCurPacket->AddItem(t9);
   sCurPacket->AddItem(t10);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11>
void Packetize(T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9, T10 t10, T11 t11)
{
   sCurPacket->AddItem(t1);
   sCurPacket->AddItem(t2);
   sCurPacket->AddItem(t3);
   sCurPacket->AddItem(t4);
   sCurPacket->AddItem(t5);
   sCurPacket->AddItem(t6);
   sCurPacket->AddItem(t7);
   sCurPacket->AddItem(t8);
   sCurPacket->AddItem(t9);
   sCurPacket->AddItem(t10);
   sCurPacket->AddItem(t11);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11,
          typename T12>
void Packetize(T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9, T10 t10, T11 t11, T12 t12)
{
   sCurPacket->AddItem(t1);
   sCurPacket->AddItem(t2);
   sCurPacket->AddItem(t3);
   sCurPacket->AddItem(t4);
   sCurPacket->AddItem(t5);
   sCurPacket->AddItem(t6);
   sCurPacket->AddItem(t7);
   sCurPacket->AddItem(t8);
   sCurPacket->AddItem(t9);
   sCurPacket->AddItem(t10);
   sCurPacket->AddItem(t11);
   sCurPacket->AddItem(t12);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11,
   typename T12, typename T13>
   void Packetize(T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9, T10 t10, T11 t11, T12 t12, T13 t13)
{
   sCurPacket->AddItem(t1);
   sCurPacket->AddItem(t2);
   sCurPacket->AddItem(t3);
   sCurPacket->AddItem(t4);
   sCurPacket->AddItem(t5);
   sCurPacket->AddItem(t6);
   sCurPacket->AddItem(t7);
   sCurPacket->AddItem(t8);
   sCurPacket->AddItem(t9);
   sCurPacket->AddItem(t10);
   sCurPacket->AddItem(t11);
   sCurPacket->AddItem(t12);
   sCurPacket->AddItem(t13);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11,
   typename T12, typename T13, typename T14>
   void Packetize(T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9, T10 t10, T11 t11, T12 t12, T13 t13, T14 t14)
{
   sCurPacket->AddItem(t1);
   sCurPacket->AddItem(t2);
   sCurPacket->AddItem(t3);
   sCurPacket->AddItem(t4);
   sCurPacket->AddItem(t5);
   sCurPacket->AddItem(t6);
   sCurPacket->AddItem(t7);
   sCurPacket->AddItem(t8);
   sCurPacket->AddItem(t9);
   sCurPacket->AddItem(t10);
   sCurPacket->AddItem(t11);
   sCurPacket->AddItem(t12);
   sCurPacket->AddItem(t13);
   sCurPacket->AddItem(t14);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11,
   typename T12, typename T13, typename T14, typename T15>
   void Packetize(T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9, T10 t10, T11 t11, T12 t12, T13 t13, T14 t14, T15 t15)
{
   sCurPacket->AddItem(t1);
   sCurPacket->AddItem(t2);
   sCurPacket->AddItem(t3);
   sCurPacket->AddItem(t4);
   sCurPacket->AddItem(t5);
   sCurPacket->AddItem(t6);
   sCurPacket->AddItem(t7);
   sCurPacket->AddItem(t8);
   sCurPacket->AddItem(t9);
   sCurPacket->AddItem(t10);
   sCurPacket->AddItem(t11);
   sCurPacket->AddItem(t12);
   sCurPacket->AddItem(t13);
   sCurPacket->AddItem(t14);
   sCurPacket->AddItem(t15);
}

static uint32_t TextureSize(uint32_t w, uint32_t h, uint32_t format, uint32_t type, uint32_t unpackAlignment)
{
   uint32_t bitsPerChannel = 0;
   uint32_t channels = 0;
   uint32_t bitsPerPixel = 0;
   uint32_t bytesPerRow = 0;
   uint32_t upaMinus1 = unpackAlignment - 1;

   switch (format)
   {
   case GL_LUMINANCE :
   case GL_ALPHA :
   case GL_RED :
   case GL_RED_INTEGER :
   case GL_DEPTH_COMPONENT :
   case GL_DEPTH_STENCIL :
      channels = 1;
      break;

   case GL_LUMINANCE_ALPHA :
   case GL_RG :
   case GL_RG_INTEGER :
      channels = 2;
      break;

   case GL_RGB :
   case GL_RGB_INTEGER :
      channels = 3;
      break;

   case GL_RGBA :
   case GL_RGBA_INTEGER :
   case GL_BGRA_EXT:
      channels = 4;
      break;
   default :
      assert(0);
   }

   switch (type)
   {
   case GL_BYTE :
   case GL_UNSIGNED_BYTE :
      bitsPerChannel = 8;
      break;

   case GL_SHORT :
   case GL_UNSIGNED_SHORT :
   case GL_HALF_FLOAT :
   case GL_HALF_FLOAT_OES :
      bitsPerChannel = 16;
      break;

   case GL_UNSIGNED_SHORT_5_6_5 :
   case GL_UNSIGNED_SHORT_4_4_4_4 :
   case GL_UNSIGNED_SHORT_5_5_5_1 :
      bitsPerPixel = 16;
      break;

   case GL_INT :
   case GL_UNSIGNED_INT :
   case GL_FLOAT :
      bitsPerChannel = 32;
      break;

   case GL_UNSIGNED_INT_2_10_10_10_REV :
   case GL_UNSIGNED_INT_10F_11F_11F_REV :
   case GL_UNSIGNED_INT_5_9_9_9_REV :
   case GL_UNSIGNED_INT_24_8 :
   case GL_FLOAT_32_UNSIGNED_INT_24_8_REV :
      bitsPerPixel = 32;
      break;

   default:
      assert(0);
   }

   if (bitsPerPixel == 0)
      bitsPerPixel = bitsPerChannel * channels;

   // Round to a multiple of 8 bits per pixel
   bytesPerRow = (bitsPerPixel * w + 7) / 8;

   // Round to unpackAlignment b
   bytesPerRow = (bytesPerRow + upaMinus1) & (~upaMinus1);

   return bytesPerRow * h;
}

static uint32_t TextureSize3D(uint32_t w, uint32_t h, uint32_t depth, uint32_t format, uint32_t type, uint32_t unpackAlignment)
{
   return TextureSize(w, h, format, type, unpackAlignment) * depth;
}

static uint32_t UnpackAlignment()
{
   GLint v[4];
   Real(glGetIntegerv)(GL_UNPACK_ALIGNMENT, v);
   return v[0];
}

static Remote *remote = NULL;

static bool PostPacketize(Packet *p);
static uint32_t GetESMajorVersion();

struct SavedContextState
{
   EGLContext m_context;
   EGLSurface m_drawSurf;
   EGLSurface m_readSurf;
};

static SavedContextState SaveAndChangeContext(uint32_t newContext)
{
   EGLDisplay disp = Real(eglGetDisplay)(EGL_DEFAULT_DISPLAY);

   SavedContextState scs;
   scs.m_context  = Real(eglGetCurrentContext)();
   scs.m_drawSurf = Real(eglGetCurrentSurface)(EGL_DRAW);
   scs.m_readSurf = Real(eglGetCurrentSurface)(EGL_READ);

   Real(eglMakeCurrent)(disp, EGL_NO_SURFACE, EGL_NO_SURFACE, (EGLContext)(uintptr_t)newContext);
   return scs;
}

static void RestoreContext(const SavedContextState &scs)
{
   EGLDisplay disp = Real(eglGetDisplay)(EGL_DEFAULT_DISPLAY);
   Real(eglMakeCurrent)(disp, scs.m_drawSurf, scs.m_readSurf, scs.m_context);
}

static void GetFramebufferInfo(const Packet &packet)
{
   uint32_t fb = packet.Item(1).GetUInt32();
   uint32_t context = packet.Item(2).GetUInt32();
   EGLSurface drawSurf = (EGLSurface)(uintptr_t)packet.Item(3).GetUInt32();

   Packet dataPacket(eFRAMEBUFFER_INFO);
   dataPacket.AddItem(PacketItem(1));     // v1
   dataPacket.AddItem(PacketItem(context));
   dataPacket.AddItem(PacketItem(fb));

   // Switch context
   SavedContextState scs = SaveAndChangeContext(context);
   Real(eglMakeCurrent)(Real(eglGetDisplay)(EGL_DEFAULT_DISPLAY), drawSurf, drawSurf, (EGLContext)(uintptr_t)context);

   // Switch current FB
   GLint curFB = 0;
   Real(glGetIntegerv)(GL_FRAMEBUFFER_BINDING, &curFB);
   if (curFB != (GLint)fb)
      Real(glBindFramebuffer)(GL_FRAMEBUFFER, fb);

   bool isFramebuffer = fb == 0 || Real(glIsFramebuffer(fb));
   dataPacket.AddItem(PacketItem(isFramebuffer ? 1 : 0));

   if (isFramebuffer)
   {
      GLint val = 0;

      GLint numColAtts = 1;
      Real(glGetIntegerv)(GL_MAX_COLOR_ATTACHMENTS, &numColAtts);
      dataPacket.AddItem(PacketItem(numColAtts));

      if (fb != 0)
      {
         GLenum status = Real(glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER));
         dataPacket.AddItem(PacketItem(GL_FRAMEBUFFER_COMPLETE));
         dataPacket.AddItem(PacketItem(status));
      }

      GLenum pnames[] =
      {
         GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,
         GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,
         GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING,
         GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE,
         GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE,
         GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE,
         GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE,
         GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE,
         GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE,
         GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE,
         GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER,
         GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL,
         GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE
      };

      if (fb == 0)
      {
         GLenum attachments[] = { GL_BACK, GL_DEPTH, GL_STENCIL };

         dataPacket.AddItem(PacketItem(sizeof(attachments) / sizeof(GLenum)));

         for (GLenum a : attachments)
         {
            dataPacket.AddItem(PacketItem(sizeof(pnames) / sizeof(GLenum)));

            for (GLenum p : pnames)
            {
               val = 0;
               Real(glGetFramebufferAttachmentParameteriv(GL_DRAW_FRAMEBUFFER, a, p, &val));
               dataPacket.AddItem(PacketItem(p));
               dataPacket.AddItem(PacketItem(val));
            }
         }
      }
      else
      {
         GLenum attachments[] = { GL_DEPTH_ATTACHMENT, GL_STENCIL_ATTACHMENT, GL_DEPTH_STENCIL_ATTACHMENT };
         GLint numColAtts;

         Real(glGetIntegerv)(GL_MAX_COLOR_ATTACHMENTS, &numColAtts);

         dataPacket.AddItem(PacketItem(numColAtts + sizeof(attachments) / sizeof(GLenum)));

         for (GLint i = 0; i < numColAtts; i++)
         {
            dataPacket.AddItem(PacketItem(sizeof(pnames) / sizeof(GLenum)));

            for (GLenum p : pnames)
            {
               val = 0;
               Real(glGetFramebufferAttachmentParameteriv(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, p, &val));
               dataPacket.AddItem(PacketItem(p));
               dataPacket.AddItem(PacketItem(val));
            }
         }

         for (GLenum a : attachments)
         {
            dataPacket.AddItem(PacketItem(sizeof(pnames) / sizeof(GLenum)));

            for (GLenum p : pnames)
            {
               val = 0;
               Real(glGetFramebufferAttachmentParameteriv(GL_DRAW_FRAMEBUFFER, a, p, &val));
               dataPacket.AddItem(PacketItem(p));
               dataPacket.AddItem(PacketItem(val));
            }
         }

      }
   }

   Real(glGetError)();  // Clear any errors we may have tripped

   RestoreContext(scs);

   if (remote)
      dataPacket.Send(remote);
}

static void GetRenderbufferInfo(const Packet &packet)
{
   uint32_t rb = packet.Item(1).GetUInt32();
   uint32_t context = packet.Item(2).GetUInt32();

   Packet dataPacket(eRENDERBUFFER_INFO);
   dataPacket.AddItem(PacketItem(1));     // v1
   dataPacket.AddItem(PacketItem(context));
   dataPacket.AddItem(PacketItem(rb));

   // Switch context
   SavedContextState scs = SaveAndChangeContext(context);

   bool isRenderbuffer = Real(glIsRenderbuffer(rb));
   dataPacket.AddItem(PacketItem(isRenderbuffer ? 1 : 0));

   if (isRenderbuffer)
   {
      GLint val;
      GLenum pnames[] =
      {
         GL_RENDERBUFFER_WIDTH,
         GL_RENDERBUFFER_HEIGHT,
         GL_RENDERBUFFER_INTERNAL_FORMAT,
         GL_RENDERBUFFER_SAMPLES,
         GL_RENDERBUFFER_RED_SIZE,
         GL_RENDERBUFFER_GREEN_SIZE,
         GL_RENDERBUFFER_BLUE_SIZE,
         GL_RENDERBUFFER_ALPHA_SIZE,
         GL_RENDERBUFFER_DEPTH_SIZE,
         GL_RENDERBUFFER_STENCIL_SIZE
      };

      dataPacket.AddItem(PacketItem(sizeof(pnames) / sizeof(GLenum)));

      for (GLenum p : pnames)
      {
         Real(glGetRenderbufferParameteriv(GL_RENDERBUFFER, p, &val));
         dataPacket.AddItem(PacketItem(p));
         dataPacket.AddItem(PacketItem(val));
      }
   }

   RestoreContext(scs);

   if (remote)
      dataPacket.Send(remote);
}

static void GetGLBuffer(const Packet &packet)
{
   GLint v[4];

   v[0] = v[1] = 0;

   uint32_t fb = packet.Item(1).GetUInt32();

   EGLContext curContext = Real(eglGetCurrentContext)();
   EGLContext context = EGL_NO_CONTEXT;
   EGLSurface oDrawSurf = Real(eglGetCurrentSurface)(EGL_DRAW);
   EGLSurface oReadSurf = Real(eglGetCurrentSurface)(EGL_READ);
   EGLDisplay disp = Real(eglGetDisplay)(EGL_DEFAULT_DISPLAY);
   GLint      curFB = 0;
   GLint      curReadBuffer;
   GLint      readBuffer = GL_BACK;
   EGLSurface drawSurf = oDrawSurf;

   Real(glGetIntegerv)(GL_FRAMEBUFFER_BINDING, &curFB);

   if (packet.NumItems() > 2)
   {
      context = (EGLContext)(uintptr_t)packet.Item(2).GetUInt32();
      v[2] = packet.Item(3).GetUInt32();
      v[3] = packet.Item(4).GetUInt32();
      EGLSurface drawSurf = (EGLSurface)(uintptr_t)packet.Item(5).GetUInt32();

      // Change context
      Real(eglMakeCurrent)(disp, drawSurf, drawSurf, context);

      // And change framebuffer
      if (fb != (uint32_t)curFB)
         Real(glBindFramebuffer)(GL_FRAMEBUFFER, fb);
   }

   if (packet.NumItems() > 6) // For multiple color buffers in FBO
   {
      if (GetESMajorVersion() > 1)
      {
         readBuffer = packet.Item(6).GetInt32();

         Real(glGetIntegerv)(GL_READ_BUFFER, &curReadBuffer);

         if (readBuffer != curReadBuffer)
            Real(glReadBuffer)(readBuffer);
      }
   }

   if (fb == 0)   // Main framebuffer
   {
      EGLDisplay disp = Real(eglGetDisplay)(EGL_DEFAULT_DISPLAY);
      Real(eglQuerySurface)(disp, drawSurf, EGL_WIDTH, &v[2]);
      Real(eglQuerySurface)(disp, drawSurf, EGL_HEIGHT, &v[3]);
   }

   uint8_t *buf = NULL;
   if (v[2] != 0 && v[3] != 0)
   {
      buf = new uint8_t[v[2] * v[3] * 4];
      Real(glReadPixels)(v[0], v[1], v[2], v[3], GL_RGBA, GL_UNSIGNED_BYTE, buf);
   }

   Packet dataPacket(eBUFFER);
   dataPacket.AddItem(packet.Item(1)); // id
   dataPacket.AddItem(PacketItem(v[0]));
   dataPacket.AddItem(PacketItem(v[1]));
   dataPacket.AddItem(PacketItem(v[2]));
   dataPacket.AddItem(PacketItem(v[3]));
   dataPacket.AddItem(PacketItem(buf, v[2] * v[3] * 4));

   if (packet.NumItems() > 6)  // For multiple colour buffers in FBO
   {
      if (readBuffer != curReadBuffer)
         Real(glReadBuffer)(curReadBuffer);
   }

   // Restore changed state
   if (packet.NumItems() > 2)
   {
      Real(eglMakeCurrent)(disp, oDrawSurf, oReadSurf, curContext);

      if (fb != (uint32_t)curFB)
         Real(glBindFramebuffer)(GL_FRAMEBUFFER, curFB);

   }

   if (remote)
      dataPacket.Send(remote);

   delete [] buf;
}

static GLenum TargetToBinding(GLenum target)
{
   GLenum binding = GL_NONE;
   switch (target)
   {
   case GL_ARRAY_BUFFER:              return GL_ARRAY_BUFFER_BINDING;
   case GL_ELEMENT_ARRAY_BUFFER:      return GL_ELEMENT_ARRAY_BUFFER_BINDING;
   case GL_TRANSFORM_FEEDBACK_BUFFER: return GL_TRANSFORM_FEEDBACK_BUFFER_BINDING;
   case GL_UNIFORM_BUFFER:            return GL_UNIFORM_BUFFER_BINDING;
   case GL_ATOMIC_COUNTER_BUFFER:     return GL_ATOMIC_COUNTER_BUFFER_BINDING;
   case GL_DRAW_INDIRECT_BUFFER:      return GL_DRAW_INDIRECT_BUFFER_BINDING;
   case GL_SHADER_STORAGE_BUFFER:     return GL_SHADER_STORAGE_BUFFER_BINDING;
   case GL_DISPATCH_INDIRECT_BUFFER:  return GL_DISPATCH_INDIRECT_BUFFER_BINDING;
   case GL_PIXEL_PACK_BUFFER:         return GL_PIXEL_PACK_BUFFER_BINDING;
   case GL_PIXEL_UNPACK_BUFFER:       return GL_PIXEL_UNPACK_BUFFER_BINDING;
   case GL_COPY_READ_BUFFER:          return GL_COPY_READ_BUFFER_BINDING;
   case GL_COPY_WRITE_BUFFER:         return GL_COPY_WRITE_BUFFER_BINDING;
   }
   return binding;
}

static void GetGLBufferObjectData(const Packet &packet)
{
   uint32_t version  = packet.Item(1).GetUInt32();
   uint32_t context  = packet.Item(2).GetUInt32();
   uint32_t bufferId = packet.Item(3).GetUInt32();
   uint32_t target   = packet.Item(4).GetUInt32();
   uint32_t size     = packet.Item(5).GetUInt32();

   Packet dataPacket(eBUFFER_OBJECT_DATA);
   dataPacket.AddItem(1);        // v1 response
   dataPacket.AddItem(context);
   dataPacket.AddItem(bufferId);
   dataPacket.AddItem(target);

   GLint mapped;
   GLint oldBoundBuffer;
   void *ptr = nullptr;
   GLenum errCode;

   SavedContextState scs = SaveAndChangeContext(context);

   if (version != 1)
      goto err;

   if (bufferId == 0)
      goto err;

   Real(glGetIntegerv)(TargetToBinding(target), &oldBoundBuffer);
   if (oldBoundBuffer != 0)
   {
      Real(glGetBufferParameteriv)(target, GL_BUFFER_MAPPED, &mapped);
      errCode = Real(glGetError)();
      if (errCode != GL_NO_ERROR)
         goto err;

      if (mapped)
         goto err;
   }

   Real(glBindBuffer)(target, bufferId);

   ptr = Real(glMapBufferRange(target, 0, size, GL_MAP_READ_BIT));
   if (ptr != nullptr)
   {
      dataPacket.AddItem(size);
      dataPacket.AddItem(PacketItem(ptr, size));
   }
   else
      dataPacket.AddItem(0);

   // Send before the data gets unmapped
   if (remote)
      dataPacket.Send(remote);

   Real(glUnmapBuffer)(target);

   // Restore old binding
   Real(glBindBuffer)(target, oldBoundBuffer);

   // Clear any error condition we may have triggered
   Real(glGetError());

   RestoreContext(scs);
   return;

err:
   RestoreContext(scs);
   dataPacket.AddItem(0);
   dataPacket.Send(remote);
}

static void SendPerfDataPacket()
{
   Packet p(ePERF_DATA);
   EGLint numBytes = 0;

   p.AddItem(PacketItem(4)); // Version 4 of perf counter data
   p.AddItem(Control::ePerfGet);
   p.AddItem(sTotalFrameCnt);

   sGetPerfCounterDataBRCM(0, NULL, &numBytes, EGL_FALSE);
   uint8_t *buf = new uint8_t[numBytes];
   sGetPerfCounterDataBRCM(numBytes, buf, &numBytes, EGL_TRUE);   // TRUE - reset counters

   p.AddItem(PacketItem((uint32_t)(plGetTimeNowUs() >> 32)));
   p.AddItem(PacketItem((uint32_t)(plGetTimeNowUs() & 0xFFFFFFFF)));
   p.AddItem(PacketItem(numBytes));
   p.AddItem(PacketItem(buf, numBytes));

   PostPacketize(&p);

   delete[] buf;

   sTimeStampMs = plGetTimeNowMs();
   sFrameCnt = 0;
}

static void GetPerfData(const Packet &packet)
{
   EGLDisplay disp = Real(eglGetDisplay)(EGL_DEFAULT_DISPLAY);

   Real(eglInitialize)( disp, NULL, NULL );

   Control::ePerfAction action = (Control::ePerfAction)packet.Item(1).GetUInt32();

   sFrameCnt = 0;
   sTimeStampMs = plGetTimeNowMs();

   if (action == Control::ePerfStart)
   {
      Packet dataPacket(ePERFDATASET);
      sSetPerfCountingBRCM(EGL_ACQUIRE_COUNTERS_BRCM);

      uint32_t numGroups = packet.Item(2).GetUInt32();
      uint32_t i = 3;
      for (uint32_t g = 0; g < numGroups; g++)
      {
         uint32_t grpIndex = packet.Item(i++).GetUInt32();
         uint32_t count = packet.Item(i++).GetUInt32();

         // Disable all counters in group
         sChoosePerfCountersBRCM(EGL_FALSE, grpIndex, 0, NULL);

         if (count > 0)
         {
            EGLint *counterList = new EGLint[count];

            for (uint32_t c = 0; c < count; c++)
               counterList[c] = (EGLint)packet.Item(i++).GetUInt32();

            sChoosePerfCountersBRCM(EGL_TRUE, grpIndex, count, counterList);

            delete[] counterList;
         }
      }

      sSetPerfCountingBRCM(EGL_START_COUNTERS_BRCM);
      if (remote)
         dataPacket.Send(remote);
   }
   else if (action == Control::ePerfStop)
   {
      Packet dataPacket(ePERFDATASET);
      sSetPerfCountingBRCM(EGL_STOP_COUNTERS_BRCM);
      sSetPerfCountingBRCM(EGL_RELEASE_COUNTERS_BRCM);
      if (remote)
         dataPacket.Send(remote);
   }
   else if (action == Control::ePerfGet)
   {
      ALOGE("ERROR: gpumon_hook received legacy ePerfGet request");
   }
   else if (action == Control::ePerfNames)
   {
      Packet                     p(ePERF_DATA_NAMES);
      std::vector<char *>        strings;    // String buffers must stay alive until data is sent

      p.AddItem(PacketItem(3)); // Version 3 of perf data
      p.AddItem(action);

      // Enumerate the counters
      EGLint numPerfCounterGroups = sGetPerfCounterConstantBRCM(EGL_NUM_COUNTER_GROUPS_BRCM);
      EGLint perfCounterMaxStrLen = sGetPerfCounterConstantBRCM(EGL_MAX_COUNTER_STRING_LEN_BRCM);

      p.AddItem(perfCounterMaxStrLen);
      p.AddItem(numPerfCounterGroups);

      char *name1 = new char[perfCounterMaxStrLen];
      char *name2 = new char[perfCounterMaxStrLen];

      for (int32_t g = 0; g < numPerfCounterGroups; g++)
      {
         EGLint      numCounters;
         EGLint      maxActiveCounters;

         sGetPerfCounterGroupInfoBRCM(g, perfCounterMaxStrLen, name1, &numCounters, &maxActiveCounters);
         strings.push_back(strdup(name1));
         p.AddItem(strings.back());

         p.AddItem(numCounters);
         p.AddItem(maxActiveCounters);

         for (EGLint c = 0; c < numCounters; c++)
         {
            EGLuint64BRCM   minVal;
            EGLuint64BRCM   maxVal;
            EGLuint64BRCM   denominator;

            sGetPerfCounterInfoBRCM(g, c, &minVal, &maxVal, &denominator,
                                     perfCounterMaxStrLen, name1, perfCounterMaxStrLen, name2);

            strings.push_back(strdup(name1));
            p.AddItem(strings.back());

            strings.push_back(strdup(name2));
            p.AddItem(strings.back());

            p.AddItem((uint32_t)(minVal >> 32));
            p.AddItem((uint32_t)(minVal & 0xFFFFFFFF));

            p.AddItem((uint32_t)(maxVal >> 32));
            p.AddItem((uint32_t)(maxVal & 0xFFFFFFFF));

            p.AddItem((uint32_t)(denominator >> 32));
            p.AddItem((uint32_t)(denominator & 0xFFFFFFFF));
         }
      }

      if (remote)
         p.Send(remote);

      for (unsigned ss = 0; ss < strings.size(); ss++)
         free(strings[ss]);

      delete[] name1;
      delete[] name2;
   }
}

static void CollectLowerLevelEventData()
{
   EGLuint64BRCM  timebase;
   EGLBoolean     overflowed;
   EGLint         bytes;

   // Get the number of bytes to be read
   sGetEventDataBRCM(0, NULL, &bytes, &overflowed, &timebase);

   if (bytes > 0)
   {
      uint32_t *buffer = NULL;
      uint32_t buffer_max_size = sEventBuffer.getMaxSize();
      if ((EGLint)buffer_max_size < bytes)
      {
         EGLint bytes_read;
         uint32_t bytes_to_ignore = bytes - buffer_max_size;

         buffer = sEventBuffer.getPointerToWriteData(buffer_max_size);
         if (!buffer)
            return;
         bytes = buffer_max_size;

         while (bytes_to_ignore > 0)
         {
            if (bytes_to_ignore > buffer_max_size)
            {
               sGetEventDataBRCM(buffer_max_size, buffer, &bytes_read, &overflowed, &timebase);
               bytes_to_ignore -= buffer_max_size;
            }
            else
            {
               sGetEventDataBRCM(bytes_to_ignore, buffer, &bytes_read, &overflowed, &timebase);
               bytes_to_ignore = 0;
            }
         }
      }
      else
         buffer = sEventBuffer.getPointerToWriteData(bytes);


      if (buffer)
         sGetEventDataBRCM(bytes, buffer, &bytes, &overflowed, &timebase);

      if (overflowed)
         sEventOverflow = EGL_TRUE;
   }
}

// This function is used to create two dummy events (start and end) to fill the gap between
// the first event that has been recorded and lost, and the oldest event in the event buffer
static void UpdateBlankEventBuffer(uint32_t *buffer, const void *recentEvents, uint64_t firstAPIEventTime)
{
   // Start event
   *((uint64_t*)buffer) = firstAPIEventTime;
   buffer += 2;
   *buffer++ = sEventApiTrack;
   *buffer++ = sEventId;
   *buffer++ = sApiEventCode;
   *buffer++ = 0;
   *buffer++ = cmd_none;

   // End event
   *((uint64_t*)buffer) = *((uint64_t*)recentEvents);
   buffer += 2;
   *buffer++ = sEventApiTrack;
   *buffer++ = sEventId;
   *buffer++ = sApiEventCode;
   *buffer++ = 1;
   *buffer   = cmd_none;

   sEventId++;
}

static void GetEventData(const Packet &packet)
{
   Packet p(eEVENT_DATA);

   bool  clearBuffers = false;
   char *name = NULL;
   uint8_t *data = NULL;
   std::vector<char*> strings;    // String buffers must stay alive until data is sent

   Control::eEventAction action = (Control::eEventAction)packet.Item(1).GetUInt32();

   p.AddItem(PacketItem(3)); // Version 3 of event block data
   p.AddItem(action);

   if (action == Control::eEventNames)
   {
      EGLint numEventTracks, eventMaxStrLen, numEvents;

      eventMaxStrLen = sGetEventConstantBRCM(EGL_MAX_EVENT_STRING_LEN_BRCM);
      numEventTracks = sGetEventConstantBRCM(EGL_NUM_EVENT_TRACKS_BRCM);
      numEvents      = sGetEventConstantBRCM(EGL_NUM_EVENTS_BRCM);

      p.AddItem(eventMaxStrLen);
      p.AddItem(numEventTracks + 1);   // Plus 1 for our own API track
      p.AddItem(numEvents + 2);        // Plus 2 for our own API call event and texture API event

      if (eventMaxStrLen > 0)
         name = new char[eventMaxStrLen];

      // Adding the tracks
      for (EGLint t = 0; t < numEventTracks; t++)
      {
         sGetEventTrackInfoBRCM(t, eventMaxStrLen, name);
         strings.push_back(strdup(name));
         p.AddItem(strings.back());
      }

      // Adding an extra track
      strings.push_back(strdup("API"));
      p.AddItem(strings.back());

      for (EGLint e = 0; e < numEvents; e++)
      {
         EGLint numFields;
         sGetEventInfoBRCM(e, eventMaxStrLen, name, &numFields);

         strings.push_back(strdup(name));
         p.AddItem(strings.back());             // Add the name of the event

         p.AddItem(numFields);                  // Add the number of event fields

         for (EGLint f = 0; f < numFields; f++)
         {
            EGLBoolean  isSigned;
            EGLint      bytes;
            sGetEventDataFieldInfoBRCM(e, f, eventMaxStrLen, name, &isSigned, &bytes);

            strings.push_back(strdup(name));
            p.AddItem(strings.back());          // Add the name of the field

            p.AddItem(isSigned ? 1 : 0);        // Add signed / unsigned
            p.AddItem(bytes);                   // Add number of bytes for that field
         }
      }

      // Add our own "API call" event
      strings.push_back(strdup("API call"));
      p.AddItem(strings.back());                // Add the name of the event
      p.AddItem(1);                             // Add the number of event fields
      strings.push_back(strdup("Function"));
      p.AddItem(strings.back());                // Add the name of the field
      p.AddItem(0);                             // Add unsigned
      p.AddItem(4);                             // Add number of bytes for that field

      // Add our own "Texture API call" event
      strings.push_back(strdup("Texture API call"));
      p.AddItem(strings.back());                // Add the name of the event
      p.AddItem(3);                             // Add the number of event fields
      strings.push_back(strdup("Function"));
      p.AddItem(strings.back());                // Add the name of the field
      p.AddItem(0);                             // Add unsigned
      p.AddItem(4);                             // Add number of bytes for that field
      strings.push_back(strdup("Texture ID"));
      p.AddItem(strings.back());                // Add the name of the field
      p.AddItem(0);                             // Add unsigned
      p.AddItem(4);                             // Add number of bytes for that field
      strings.push_back(strdup("Mip Level"));
      p.AddItem(strings.back());                // Add the name of the field
      p.AddItem(0);                             // Add unsigned
      p.AddItem(4);                             // Add number of bytes for that field
   }
   else if (action == Control::eEventGet)
   {
      if (sCaptureEvents)
      {
         CollectLowerLevelEventData();

         if (sEventBuffer.size() == 0)
         {
            p.AddItem(0);
         }
         else
         {
            EGLuint64BRCM  timebase;
            EGLBoolean     overflowed;
            EGLint         bytes;

            // We're just using this to get the timebase
            sGetEventDataBRCM(0, NULL, &bytes, &overflowed, &timebase);
            uint64_t nowUs = plGetTimeNowUs();

            if (sEventBuffer.dataHasBeenLost())
               p.AddItem(sEventBuffer.size() + BLANK_EVENTS_BUFFER_SIZE);
            else
               p.AddItem(sEventBuffer.size());

            p.AddItem((uint32_t)(timebase >> 32));
            p.AddItem((uint32_t)(timebase & 0xFFFFFFFF));
            p.AddItem((uint32_t)(nowUs >> 32));
            p.AddItem((uint32_t)(nowUs & 0xFFFFFFFF));
            p.AddItem(sEventOverflow);

            uint32_t numArrays = 0;
            if (sEventBuffer.size() > 0)
            {
               numArrays++;
               uint32_t size = 0;
               sEventBuffer.getSecondBufferPart(&size);
               if (size != 0)
                  numArrays++;
               if (sEventBuffer.dataHasBeenLost())
                  numArrays++;   // a blank event will be added to cover the lost data
            }

            p.AddItem(numArrays);
            // The first event in this buffer should have the smallest time stamp - this is only
            // needed to set the timestamp of the start of the recorded range in GPUMonitor.
            // The events can be in any order after that.
            if (sEventBuffer.size() > 0)
            {
               uint32_t size = 0;
               void *data = sEventBuffer.getFirstBufferPart(&size);

               // If some data has been lost add a blank event
               // lasting between the first API call and the
               // oldest event in the buffer
               if (sEventBuffer.dataHasBeenLost())
               {
                  UpdateBlankEventBuffer(reinterpret_cast<uint32_t *> (sBlankEventBuffer), data, sFirstApiCapTime);
                  p.AddItem(PacketItem(sBlankEventBuffer, BLANK_EVENTS_BUFFER_SIZE));
               }

               p.AddItem(PacketItem(data, size));
               data = sEventBuffer.getSecondBufferPart(&size);
               if (size != 0)
                  p.AddItem(PacketItem(data, size));
            }

            sEventOverflow = false;
            clearBuffers = true;
         }
      }
      else
      {
         p.AddItem(0);
      }
   }

   if (remote)
      p.Send(remote);

   if (clearBuffers)
      sEventBuffer.reset();

   for (unsigned ss = 0; ss < strings.size(); ss++)
      free(strings[ss]);

   if (name)
      delete[] name;

   if (data)
      delete[] data;
}

static void GetUniforms(const Packet &packet)
{
   uint32_t progId = packet.Item(1).GetUInt32();
   uint32_t context = (uint32_t)(uintptr_t)Real(eglGetCurrentContext());

   if (packet.NumItems() > 2)
      context = packet.Item(2).GetUInt32();

   Packet dataPacket(eUNIFORMS);
   dataPacket.AddItem(PacketItem(2));     // v2
   dataPacket.AddItem(PacketItem(context));
   dataPacket.AddItem(PacketItem(progId));

   SavedContextState scs = SaveAndChangeContext(context);

   dataPacket.AddItem(PacketItem((GLuint)Real(glIsProgram(progId))));

   GLint activeUniforms, linkStatus, maxLen, infoLen;
   char **nameBufs = 0;
   char  *logBuf = 0;
   char  *shLogBuf = 0;

   if (Real(glIsProgram)(progId))
   {
      Real(glGetProgramiv)(progId, GL_LINK_STATUS, &linkStatus);
      Real(glGetProgramiv)(progId, GL_ACTIVE_UNIFORMS, &activeUniforms);
      Real(glGetProgramiv)(progId, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxLen);

      dataPacket.AddItem(PacketItem(linkStatus));
      dataPacket.AddItem(PacketItem(activeUniforms));

      Real(glGetProgramiv)(progId, GL_INFO_LOG_LENGTH, &infoLen);
      dataPacket.AddItem(PacketItem(infoLen));
      if (infoLen > 0)
      {
         logBuf = new char[infoLen];
         Real(glGetProgramInfoLog)(progId, infoLen, NULL, logBuf);
         dataPacket.AddItem(PacketItem(logBuf, infoLen));
      }

      if (activeUniforms > 0)
         nameBufs = new char*[activeUniforms];

      for (GLint u = 0; u < activeUniforms; u++)
      {
         GLint  size;
         GLenum type;
         uint32_t c;

         nameBufs[u] = new char[maxLen];
         Real(glGetActiveUniform)(progId, u, maxLen, NULL, &size, &type, nameBufs[u]);
         GLint location = Real(glGetUniformLocation)(progId, nameBufs[u]);

         // TODO: if location == -1, the uniform is part of a uniform block and its value must be queried
         // differently - but I haven't figured out how yet

         dataPacket.AddItem(PacketItem(nameBufs[u], strlen(nameBufs[u])));
         dataPacket.AddItem(PacketItem(location));
         dataPacket.AddItem(PacketItem(type));
         dataPacket.AddItem(PacketItem(size));

         GLfloat f[16];
         GLint   i[4];

         memset(f, 0, sizeof(f));
         memset(i, 0, sizeof(i));

         switch (type)
         {
         case GL_FLOAT:
            if (location != -1)
               Real(glGetUniformfv)(progId, location, f);
            dataPacket.AddItem(PacketItem(f[0]));
            break;
         case GL_FLOAT_VEC2:
            if (location != -1)
               Real(glGetUniformfv)(progId, location, f);
            for (c = 0; c < 2; c++)
               dataPacket.AddItem(PacketItem(f[c]));
            break;
         case GL_FLOAT_VEC3:
            if (location != -1)
               Real(glGetUniformfv)(progId, location, f);
            for (c = 0; c < 3; c++)
               dataPacket.AddItem(PacketItem(f[c]));
            break;
         case GL_FLOAT_VEC4:
            if (location != -1)
               Real(glGetUniformfv)(progId, location, f);
            for (c = 0; c < 4; c++)
               dataPacket.AddItem(PacketItem(f[c]));
            break;
         case GL_INT:
         case GL_BOOL:
         case GL_UNSIGNED_INT:
            if (location != -1)
               Real(glGetUniformiv)(progId, location, i);
            dataPacket.AddItem(PacketItem(i[0]));
            break;
         case GL_INT_VEC2:
         case GL_UNSIGNED_INT_VEC2:
         case GL_BOOL_VEC2:
            if (location != -1)
               Real(glGetUniformiv)(progId, location, i);
            for (c = 0; c < 2; c++)
               dataPacket.AddItem(PacketItem(i[c]));
            break;
         case GL_INT_VEC3:
         case GL_UNSIGNED_INT_VEC3:
         case GL_BOOL_VEC3:
            if (location != -1)
               Real(glGetUniformiv)(progId, location, i);
            for (c = 0; c < 3; c++)
               dataPacket.AddItem(PacketItem(i[c]));
            break;
         case GL_INT_VEC4:
         case GL_UNSIGNED_INT_VEC4:
         case GL_BOOL_VEC4:
            if (location != -1)
               Real(glGetUniformiv)(progId, location, i);
            for (c = 0; c < 4; c++)
               dataPacket.AddItem(PacketItem(i[c]));
            break;
         case GL_FLOAT_MAT2:
            if (location != -1)
               Real(glGetUniformfv)(progId, location, f);
            for (c = 0; c < 4; c++)
               dataPacket.AddItem(PacketItem(f[c]));
            break;
         case GL_FLOAT_MAT2x3:
         case GL_FLOAT_MAT3x2:
            if (location != -1)
               Real(glGetUniformfv)(progId, location, f);
            for (c = 0; c < 6; c++)
               dataPacket.AddItem(PacketItem(f[c]));
            break;
         case GL_FLOAT_MAT2x4:
         case GL_FLOAT_MAT4x2:
            if (location != -1)
               Real(glGetUniformfv)(progId, location, f);
            for (c = 0; c < 8; c++)
               dataPacket.AddItem(PacketItem(f[c]));
            break;
         case GL_FLOAT_MAT3x4:
         case GL_FLOAT_MAT4x3:
            if (location != -1)
               Real(glGetUniformfv)(progId, location, f);
            for (c = 0; c < 12; c++)
               dataPacket.AddItem(PacketItem(f[c]));
            break;
         case GL_FLOAT_MAT3:
            if (location != -1)
               Real(glGetUniformfv)(progId, location, f);
            for (c = 0; c < 9; c++)
               dataPacket.AddItem(PacketItem(f[c]));
            break;
         case GL_FLOAT_MAT4:
            if (location != -1)
               Real(glGetUniformfv)(progId, location, f);
            for (c = 0; c < 16; c++)
               dataPacket.AddItem(PacketItem(f[c]));
            break;
         case GL_SAMPLER_2D:
         case GL_SAMPLER_2D_SHADOW:
         case GL_SAMPLER_2D_ARRAY:
         case GL_SAMPLER_2D_ARRAY_SHADOW:
         case GL_SAMPLER_3D:
         case GL_SAMPLER_CUBE:
         case GL_SAMPLER_CUBE_SHADOW:
         case GL_INT_SAMPLER_2D:
         case GL_INT_SAMPLER_2D_ARRAY:
         case GL_INT_SAMPLER_3D:
         case GL_INT_SAMPLER_CUBE:
         case GL_UNSIGNED_INT_SAMPLER_2D:
         case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
         case GL_UNSIGNED_INT_SAMPLER_3D:
         case GL_UNSIGNED_INT_SAMPLER_CUBE:
            if (location != -1)
               Real(glGetUniformiv)(progId, location, i);
            dataPacket.AddItem(PacketItem(i[0]));
            break;
         }
      }
   }

   RestoreContext(scs);

   if (remote)
      dataPacket.Send(remote);

   if (nameBufs)
   {
      for (GLint u = 0; u < activeUniforms; u++)
         delete [] nameBufs[u];

      delete [] nameBufs;
   }

   if (logBuf)
      delete [] logBuf;

   if (shLogBuf)
      delete [] shLogBuf;
}

static void GetPIQData(const Packet &packet)
{
   GLenum ifaces[] =
   {
      GL_UNIFORM,
      GL_UNIFORM_BLOCK,
      GL_ATOMIC_COUNTER_BUFFER,
      GL_PROGRAM_INPUT,
      GL_PROGRAM_OUTPUT,
      GL_TRANSFORM_FEEDBACK_VARYING,
      GL_BUFFER_VARIABLE,
      GL_SHADER_STORAGE_BLOCK
   };

   GLenum uProps[] =
   {
      GL_TYPE,
      GL_ARRAY_STRIDE, GL_BLOCK_INDEX, GL_IS_ROW_MAJOR, GL_MATRIX_STRIDE,
      GL_ATOMIC_COUNTER_BUFFER_INDEX,
      GL_LOCATION,
      GL_OFFSET,
      GL_REFERENCED_BY_COMPUTE_SHADER, GL_REFERENCED_BY_VERTEX_SHADER, GL_REFERENCED_BY_FRAGMENT_SHADER,
   };

   GLenum ubProps[] =
   {
      // If present, GL_NUM_ACTIVE_VARIABLES must be first
      GL_NUM_ACTIVE_VARIABLES, GL_BUFFER_BINDING,
      GL_BUFFER_DATA_SIZE,
      GL_REFERENCED_BY_COMPUTE_SHADER, GL_REFERENCED_BY_VERTEX_SHADER, GL_REFERENCED_BY_FRAGMENT_SHADER
   };

   GLenum acbProps[] =
   {
      // If present, GL_NUM_ACTIVE_VARIABLES must be first
      GL_NUM_ACTIVE_VARIABLES, GL_BUFFER_BINDING,
      GL_BUFFER_DATA_SIZE,
      GL_REFERENCED_BY_COMPUTE_SHADER, GL_REFERENCED_BY_VERTEX_SHADER, GL_REFERENCED_BY_FRAGMENT_SHADER
   };

   GLenum piProps[] =
   {
      GL_TYPE,
      GL_ARRAY_SIZE,
      GL_LOCATION,
      GL_REFERENCED_BY_COMPUTE_SHADER, GL_REFERENCED_BY_VERTEX_SHADER, GL_REFERENCED_BY_FRAGMENT_SHADER
   };

   GLenum poProps[] =
   {
      GL_TYPE,
      GL_ARRAY_SIZE,
      GL_LOCATION,
      GL_REFERENCED_BY_COMPUTE_SHADER, GL_REFERENCED_BY_VERTEX_SHADER, GL_REFERENCED_BY_FRAGMENT_SHADER
   };

   GLenum tffProps[] =
   {
      GL_TYPE,
      GL_ARRAY_SIZE
   };

   GLenum bvProps[] =
   {
      GL_TYPE,
      GL_ARRAY_SIZE,
      GL_ARRAY_STRIDE, GL_BLOCK_INDEX, GL_IS_ROW_MAJOR, GL_MATRIX_STRIDE,
      GL_OFFSET,
      GL_TOP_LEVEL_ARRAY_SIZE, GL_TOP_LEVEL_ARRAY_STRIDE,
      GL_REFERENCED_BY_COMPUTE_SHADER, GL_REFERENCED_BY_VERTEX_SHADER, GL_REFERENCED_BY_FRAGMENT_SHADER
   };

   GLenum ssbProps[] =
   {
      // If present, GL_NUM_ACTIVE_VARIABLES must be first
      GL_NUM_ACTIVE_VARIABLES, GL_BUFFER_BINDING,
      GL_BUFFER_DATA_SIZE,
      GL_REFERENCED_BY_COMPUTE_SHADER, GL_REFERENCED_BY_VERTEX_SHADER, GL_REFERENCED_BY_FRAGMENT_SHADER
   };

#define SZ(a) (sizeof(a) / sizeof(GLenum))

   GLenum   *props[]    = { uProps, ubProps, acbProps, piProps, poProps, tffProps, bvProps, ssbProps };
   uint32_t propsSize[] = { SZ(uProps), SZ(ubProps), SZ(acbProps), SZ(piProps),
                             SZ(poProps), SZ(tffProps), SZ(bvProps), SZ(ssbProps) };

   uint32_t progId = packet.Item(1).GetUInt32();
   uint32_t context = packet.Item(2).GetUInt32();

   Packet dataPacket(ePIQ_DATA);
   dataPacket.AddItem(PacketItem(1));     // v1
   dataPacket.AddItem(PacketItem(context));
   dataPacket.AddItem(PacketItem(progId));

   SavedContextState scs = SaveAndChangeContext(context);

   dataPacket.AddItem(PacketItem((GLuint)Real(glIsProgram(progId))));

   std::string *names = nullptr;

   if (Real(glIsProgram)(progId))
   {
      dataPacket.AddItem(sizeof(ifaces) / sizeof(GLenum));

      // Work out how many strings we will need
      GLint numStrings = 0;
      for (unsigned i = 0; i < sizeof(ifaces) / sizeof(GLenum); i++)
      {
         GLint num = 0;
         Real(glGetProgramInterfaceiv)(progId, ifaces[i], GL_ACTIVE_RESOURCES, &num);
         numStrings += num;
      }

      names = new std::string[numStrings];
      uint32_t    curStr = 0;

      for (unsigned i = 0; i < sizeof(ifaces) / sizeof(GLenum); i++)
      {
         GLint   num = 0;
         GLint   maxLen = 0;
         GLint   maxNumActive = 0;
         GLsizei length = 0;

         dataPacket.AddItem(ifaces[i]);

         Real(glGetProgramInterfaceiv)(progId, ifaces[i], GL_ACTIVE_RESOURCES, &num);

         dataPacket.AddItem(num);

         if (ifaces[i] != GL_ATOMIC_COUNTER_BUFFER)
            Real(glGetProgramInterfaceiv)(progId, ifaces[i], GL_MAX_NAME_LENGTH, &maxLen);

         dataPacket.AddItem(maxLen);

         if (ifaces[i] == GL_ATOMIC_COUNTER_BUFFER ||
             ifaces[i] == GL_SHADER_STORAGE_BLOCK ||
             ifaces[i] == GL_UNIFORM_BLOCK)
            Real(glGetProgramInterfaceiv)(progId, ifaces[i], GL_MAX_NUM_ACTIVE_VARIABLES, &maxNumActive);

         dataPacket.AddItem(maxNumActive);

         GLchar *nameBuf = new GLchar[maxLen];

         for (GLint n = 0; n < num; n++)
         {
            // Add the resource name
            Real(glGetProgramResourceName)(progId, ifaces[i], n, maxLen, &length, nameBuf);
            names[curStr] = std::string(nameBuf);
            dataPacket.AddItem(names[curStr].c_str());
            curStr++;

            // And the props
            GLint params[20];

            Real(glGetProgramResourceiv)(progId, ifaces[i], n, propsSize[i], props[i],
                                         20, &length, params);
            dataPacket.AddItem(length);
            for (int j = 0; j < length; j++)
            {
               dataPacket.AddItem(props[i][j]);
               dataPacket.AddItem(params[j]);
            }

            if (props[i][0] == GL_NUM_ACTIVE_VARIABLES)
            {
               GLint vars[128];
               GLenum activeVars[1] = { GL_ACTIVE_VARIABLES };

               Real(glGetProgramResourceiv)(progId, ifaces[i], n, 1, activeVars,
                                            128, &length, vars);

               dataPacket.AddItem(length);
               for (int k = 0; k < length; k++)
                  dataPacket.AddItem(vars[k]);
            }
            else
               dataPacket.AddItem(0);
         }

         delete [] nameBuf;
      }
   }

   RestoreContext(scs);

   if (remote)
      dataPacket.Send(remote);

   if (names)
      delete [] names;
}

static void GetSyncObjData(const Packet &packet)
{
   GLsync   syncId = (GLsync)packet.Item(1).GetVoidPtr();
   uint32_t context = packet.Item(2).GetUInt32();

   Packet dataPacket(eSYNC_OBJ_DATA);
   dataPacket.AddItem(PacketItem(1));     // v1
   dataPacket.AddItem(PacketItem(context));
   dataPacket.AddItem(PacketItem(syncId));

   SavedContextState scs = SaveAndChangeContext(context);

   bool isSync = Real(glIsSync(syncId));
   dataPacket.AddItem(PacketItem(isSync ? 1 : 0));

   if (isSync)
   {
      GLint val;

      Real(glGetSynciv)(syncId, GL_OBJECT_TYPE, 1, NULL, &val);
      dataPacket.AddItem(PacketItem(GL_OBJECT_TYPE));
      dataPacket.AddItem(PacketItem(val));

      Real(glGetSynciv)(syncId, GL_SYNC_CONDITION, 1, NULL, &val);
      dataPacket.AddItem(PacketItem(GL_SYNC_CONDITION));
      dataPacket.AddItem(PacketItem(val));

      Real(glGetSynciv)(syncId, GL_SYNC_FLAGS, 1, NULL, &val);
      dataPacket.AddItem(PacketItem(GL_SYNC_FLAGS));
      dataPacket.AddItem(PacketItem(val));

      Real(glGetSynciv)(syncId, GL_SYNC_STATUS, 1, NULL, &val);
      dataPacket.AddItem(PacketItem(GL_SYNC_STATUS));
      dataPacket.AddItem(PacketItem(val));
   }

   RestoreContext(scs);

   if (remote)
      dataPacket.Send(remote);
}

static void GetProgramPipelineData(const Packet &packet)
{
   uint32_t ppId = packet.Item(1).GetUInt32();
   uint32_t context = packet.Item(2).GetUInt32();

   Packet dataPacket(ePROGRAM_PIPELINE_DATA);
   dataPacket.AddItem(PacketItem(1));     // v1
   dataPacket.AddItem(PacketItem(context));
   dataPacket.AddItem(PacketItem(ppId));

   SavedContextState scs = SaveAndChangeContext(context);

   bool isPP = Real(glIsProgramPipeline(ppId));
   dataPacket.AddItem(PacketItem(isPP ? 1 : 0));

   GLint major, minor;
   Real(glGetIntegerv)(GL_MAJOR_VERSION, &major);
   Real(glGetIntegerv)(GL_MINOR_VERSION, &minor);

   GLint ver = major * 10 + minor;
   char  *infoLog = nullptr;

   dataPacket.AddItem(PacketItem(major));
   dataPacket.AddItem(PacketItem(minor));

   if (isPP && ver >= 31)
   {
      GLint val;

      Real(glGetProgramPipelineiv)(ppId, GL_ACTIVE_PROGRAM, &val);
      dataPacket.AddItem(PacketItem(GL_ACTIVE_PROGRAM));
      dataPacket.AddItem(PacketItem(val));

      Real(glGetProgramPipelineiv)(ppId, GL_VALIDATE_STATUS, &val);
      dataPacket.AddItem(PacketItem(GL_VALIDATE_STATUS));
      dataPacket.AddItem(PacketItem(val));

      Real(glGetProgramPipelineiv)(ppId, GL_INFO_LOG_LENGTH, &val);
      dataPacket.AddItem(PacketItem(GL_INFO_LOG_LENGTH));
      dataPacket.AddItem(PacketItem(val));

      if (val > 0)
      {
         infoLog = new char[val];
         Real(glGetProgramPipelineInfoLog)(ppId, val, nullptr, infoLog);
         dataPacket.AddItem(PacketItem(infoLog, val));
      }

      Real(glGetProgramPipelineiv)(ppId, GL_VERTEX_SHADER, &val);
      dataPacket.AddItem(PacketItem(GL_VERTEX_SHADER));
      dataPacket.AddItem(PacketItem(val));

      Real(glGetProgramPipelineiv)(ppId, GL_FRAGMENT_SHADER, &val);
      dataPacket.AddItem(PacketItem(GL_FRAGMENT_SHADER));
      dataPacket.AddItem(PacketItem(val));

      Real(glGetProgramPipelineiv)(ppId, GL_COMPUTE_SHADER, &val);
      dataPacket.AddItem(PacketItem(GL_COMPUTE_SHADER));
      dataPacket.AddItem(PacketItem(val));

      if (ver >= 32)
      {
         Real(glGetProgramPipelineiv)(ppId, GL_TESS_CONTROL_SHADER, &val);
         dataPacket.AddItem(PacketItem(GL_TESS_CONTROL_SHADER));
         dataPacket.AddItem(PacketItem(val));

         Real(glGetProgramPipelineiv)(ppId, GL_TESS_EVALUATION_SHADER, &val);
         dataPacket.AddItem(PacketItem(GL_TESS_EVALUATION_SHADER));
         dataPacket.AddItem(PacketItem(val));

         Real(glGetProgramPipelineiv)(ppId, GL_GEOMETRY_SHADER, &val);
         dataPacket.AddItem(PacketItem(GL_GEOMETRY_SHADER));
         dataPacket.AddItem(PacketItem(val));
      }
   }

   RestoreContext(scs);

   if (remote)
      dataPacket.Send(remote);

   if (infoLog != nullptr)
      delete [] infoLog;
}

static void AddInfoStr(std::vector<std::string> &strs, const char *s)
{
   if (s != NULL)
      strs.push_back(std::string(s));
   else
      strs.push_back(std::string(""));
}

static void GetInfoData(const Packet &packet)
{
   uint32_t context = packet.Item(1).GetUInt32();

   Packet dataPacket(eINFO_DATA);
   dataPacket.AddItem(PacketItem(1));     // v1
   dataPacket.AddItem(PacketItem(context));

   SavedContextState scs = SaveAndChangeContext(context);

   std::vector<std::string> strs;

   // Add the GL version
   GLint major = 0, minor = 0;
   Real(glGetIntegerv)(GL_MAJOR_VERSION, &major);
   Real(glGetIntegerv)(GL_MINOR_VERSION, &minor);

   dataPacket.AddItem(PacketItem(major));
   dataPacket.AddItem(PacketItem(minor));

   // Add the gl strings
   AddInfoStr(strs, (const char *)Real(glGetString)(GL_VENDOR));
   dataPacket.AddItem(PacketItem(strs.back().c_str()));

   AddInfoStr(strs, (const char *)Real(glGetString)(GL_RENDERER));
   dataPacket.AddItem(PacketItem(strs.back().c_str()));

   AddInfoStr(strs, (const char *)Real(glGetString)(GL_VERSION));
   dataPacket.AddItem(PacketItem(strs.back().c_str()));

   AddInfoStr(strs, (const char *)Real(glGetString)(GL_SHADING_LANGUAGE_VERSION));
   dataPacket.AddItem(PacketItem(strs.back().c_str()));

   // Add the GL extensions
   GLint numExts = 0;
   Real(glGetIntegerv)(GL_NUM_EXTENSIONS, &numExts);
   dataPacket.AddItem(PacketItem(numExts));
   for (GLint i = 0; i < numExts; i++)
   {
      AddInfoStr(strs, (const char *)Real(glGetStringi)(GL_EXTENSIONS, i));
      dataPacket.AddItem(PacketItem(strs.back().c_str()));
   }

   // Add the EGL strings
   EGLDisplay disp = Real(eglGetDisplay(EGL_DEFAULT_DISPLAY));

   AddInfoStr(strs, (const char *)Real(eglQueryString(disp, EGL_VENDOR)));
   dataPacket.AddItem(PacketItem(strs.back().c_str()));

   AddInfoStr(strs, (const char *)Real(eglQueryString(disp, EGL_VERSION)));
   dataPacket.AddItem(PacketItem(strs.back().c_str()));

   AddInfoStr(strs, (const char *)Real(eglQueryString(disp, EGL_CLIENT_APIS)));
   dataPacket.AddItem(PacketItem(strs.back().c_str()));

   // And EGL display extensions
   AddInfoStr(strs, (const char *)Real(eglQueryString(disp, EGL_EXTENSIONS)));
   dataPacket.AddItem(PacketItem(strs.back().c_str()));

   // And EGL client extensions
   AddInfoStr(strs, (const char *)Real(eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS)));
   dataPacket.AddItem(PacketItem(strs.back().c_str()));

   RestoreContext(scs);

   if (remote)
      dataPacket.Send(remote);
}

static void GetQueryObjData(const Packet &packet)
{
   uint32_t queryId = packet.Item(1).GetUInt32();
   uint32_t context = packet.Item(2).GetUInt32();

   Packet dataPacket(eQUERY_OBJ_DATA);
   dataPacket.AddItem(PacketItem(1));     // v1
   dataPacket.AddItem(PacketItem(context));
   dataPacket.AddItem(PacketItem(queryId));

   SavedContextState scs = SaveAndChangeContext(context);

   bool isQuery = Real(glIsQuery(queryId));
   dataPacket.AddItem(PacketItem(isQuery ? 1 : 0));

   if (isQuery)
   {
      GLint  id;
      GLuint val;
      GLenum target = GL_NONE;

      Real(glGetQueryiv)(GL_ANY_SAMPLES_PASSED, GL_CURRENT_QUERY, &id);

      if (id == (GLint)queryId)
         target = GL_ANY_SAMPLES_PASSED;
      else
      {
         Real(glGetQueryiv)(GL_ANY_SAMPLES_PASSED_CONSERVATIVE, GL_CURRENT_QUERY, &id);
         if (id == (GLint)queryId)
            target = GL_ANY_SAMPLES_PASSED_CONSERVATIVE;
         else
         {
            Real(glGetQueryiv)(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, GL_CURRENT_QUERY, &id);
            if (id == (GLint)queryId)
               target = GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN;
         }
      }

      dataPacket.AddItem(PacketItem(target));

      // Can only interrogate queries that aren't currently active
      if (target == GL_NONE)
      {
         val = 0;
         Real(glGetQueryObjectuiv)(queryId, GL_QUERY_RESULT_AVAILABLE, &val);
         dataPacket.AddItem(PacketItem(GL_QUERY_RESULT_AVAILABLE));
         dataPacket.AddItem(PacketItem(val));

         if (val != 0)
         {
            Real(glGetQueryObjectuiv)(queryId, GL_QUERY_RESULT, &val);
            dataPacket.AddItem(PacketItem(GL_QUERY_RESULT));
            dataPacket.AddItem(PacketItem(val));
         }
      }
   }

   RestoreContext(scs);

   if (remote)
      dataPacket.Send(remote);
}

static void AddVAOParam(Packet *dataPacket, GLint index, GLenum param)
{
   GLint p;
   Real(glGetVertexAttribiv)(index, param, &p);
   dataPacket->AddItem(param);
   dataPacket->AddItem(p);
}

static void GetVertexArrayObjData(const Packet &packet)
{
   uint32_t vaoId = packet.Item(1).GetUInt32();
   uint32_t context = packet.Item(2).GetUInt32();

   Packet dataPacket(eVERTEX_ARRAY_OBJ_DATA);
   dataPacket.AddItem(PacketItem(1));     // v1
   dataPacket.AddItem(PacketItem(context));
   dataPacket.AddItem(PacketItem(vaoId));

   SavedContextState scs = SaveAndChangeContext(context);

   bool isVao;

   // Zero is valid to query default vertex state
   if (vaoId == 0)
      isVao = true;
   else
      isVao = Real(glIsVertexArray(vaoId));

   dataPacket.AddItem(PacketItem(isVao ? 1 : 0));

   if (isVao)
   {
      GLint    maxAttribs;
      GLint    curBinding = 0;

      // Save current binding and then change
      Real(glGetIntegerv)(GL_VERTEX_ARRAY_BINDING, &curBinding);
      Real(glBindVertexArray)(vaoId);

      Real(glGetIntegerv)(GL_MAX_VERTEX_ATTRIBS, &maxAttribs);
      dataPacket.AddItem(maxAttribs);

      for (GLint i = 0; i < maxAttribs; i++)
      {
         dataPacket.AddItem(11); // 11 pairs to follow

         AddVAOParam(&dataPacket, i, GL_VERTEX_ATTRIB_BINDING);
         AddVAOParam(&dataPacket, i, GL_VERTEX_ATTRIB_ARRAY_ENABLED);
         AddVAOParam(&dataPacket, i, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING);
         AddVAOParam(&dataPacket, i, GL_VERTEX_ATTRIB_ARRAY_SIZE);
         AddVAOParam(&dataPacket, i, GL_VERTEX_ATTRIB_ARRAY_STRIDE);
         AddVAOParam(&dataPacket, i, GL_VERTEX_ATTRIB_ARRAY_TYPE);
         AddVAOParam(&dataPacket, i, GL_VERTEX_ATTRIB_ARRAY_NORMALIZED);
         AddVAOParam(&dataPacket, i, GL_VERTEX_ATTRIB_ARRAY_INTEGER);
         AddVAOParam(&dataPacket, i, GL_VERTEX_ATTRIB_ARRAY_DIVISOR);
         AddVAOParam(&dataPacket, i, GL_VERTEX_ATTRIB_RELATIVE_OFFSET);

         GLvoid *p;
         Real(glGetVertexAttribPointerv)(i, GL_VERTEX_ATTRIB_ARRAY_POINTER, &p);
         dataPacket.AddItem(GL_VERTEX_ATTRIB_ARRAY_POINTER);
         dataPacket.AddItem(p);
      }

      // Restore current binding
      Real(glBindVertexArray)(curBinding);
   }

   RestoreContext(scs);

   if (remote)
      dataPacket.Send(remote);
}

static void GetState(const Packet &packet)
{
   uint32_t context   = packet.Item(1).GetUInt32();
   uint32_t numStates = packet.Item(2).GetUInt32();

   Packet dataPacket(eSTATE);
   dataPacket.AddItem(PacketItem(numStates));

   GLint     iv[128];
   GLfloat   fv[128];
   GLboolean bv[128];
   uint32_t  j, k = 3;

   SavedContextState scs = SaveAndChangeContext(context);

   for (uint32_t i = 0; i < numStates; i++)
   {
      uint32_t type = packet.Item(k++).GetUInt32();
      uint32_t num  = packet.Item(k++).GetUInt32();
      uint32_t enm  = packet.Item(k++).GetUInt32();

      dataPacket.AddItem(PacketItem(type));
      dataPacket.AddItem(PacketItem(num));
      dataPacket.AddItem(PacketItem(enm));

      switch (type)
      {
      case 0:  memset(iv, 0, num * sizeof(GLint));
               Real(glGetIntegerv(enm, iv));
               for (j = 0; j < num; j++)
                  dataPacket.AddItem(PacketItem(iv[j]));
               break;
      case 1:  memset(fv, 0, num * sizeof(GLfloat));
               Real(glGetFloatv(enm, fv));
               for (j = 0; j < num; j++)
                  dataPacket.AddItem(PacketItem(fv[j]));
               break;
      case 2:  memset(bv, 0, num * sizeof(GLboolean));
               Real(glGetBooleanv(enm, bv));
               for (j = 0; j < num; j++)
                  dataPacket.AddItem(PacketItem((GLuint)bv[j]));
               break;
      case 3:  dataPacket.AddItem(PacketItem((GLuint)Real(glIsEnabled(enm))));
               break;
      }
   }

   if (remote)
      dataPacket.Send(remote);

   RestoreContext(scs);
}

static void GetMemory(const Packet &packet)
{
   void     *addr = packet.Item(1).GetVoidPtr();
   uint32_t numBytes = packet.Item(2).GetUInt32();

   if (numBytes == 0)
   {
      // We actually want a NULL terminated array at this pointer, so find out how big this will be
      uint8_t *ptr = (uint8_t*)addr;
      while (*ptr != 0)
      {
         ptr++;
         numBytes++;
      }
      numBytes++; // Include the NULL
   }
   else if (numBytes == 0xFFFFFFFF)
   {
      // We actually want a NULL terminated attrib array at this pointer (Odd word NULL)
      uint32_t *ptr = (uint32_t*)addr;
      bool     odd = true;
      while (!odd || *ptr != 0)
      {
         ptr++;
         numBytes += 4;
         odd = !odd;
      }
      numBytes += 4; // Include the NULL
   }

   Packet dataPacket(eMEMORY);
   dataPacket.AddItem(PacketItem(addr));
   dataPacket.AddItem(PacketItem(numBytes));
   dataPacket.AddItem(PacketItem(addr, numBytes));

   if (remote)
      dataPacket.Send(remote);
}

static void CtrlCHandler(int dummy)
{
   if (sCaptureArchive != NULL)
   {
      sCaptureArchive->Flush();
      sCaptureArchive->Disconnect();
   }

   if (remote != NULL)
      remote->Disconnect();

   exit(0);
}

static void SetCapture(bool tf, char * processName)
{
   sCaptureStream = tf;

   if (sCaptureStream && sCaptureArchive == NULL)
   {
      uint32_t maj = CAPTURE_MAJOR_VER;
      uint32_t min = CAPTURE_MINOR_VER;
      uint32_t ident = 0xBCCA97DA;
      uint32_t ptrsize = sizeof(void*);

      char     file[256];
      char    *cappath = NULL;
#ifndef ANDROID
      cappath = getenv("GPUMonitorCapturePathName");
#endif

      // If capture path specified, use that
      if (cappath != NULL && cappath[0] != '\0')
      {
         snprintf(file, sizeof(file), "%s/%s_%d.mon", cappath,
                  (processName ? processName : "bcmcapture"), getpid());
      }
      else
      {
#ifdef ANDROID
         // Only valid place to save the file is in 'our' data area
         if (processName != NULL)
         {
            snprintf(file, sizeof(file), "/data/data/%s/bcmcapture_%d.mon",
                     processName, getpid());
         }
         else
         {
            // This should never happen on Android, should it?
            ALOGE("Unable to determine where to save Capture file!");
            sCaptureStream = false;
            return;
         }
#else
         // Assume we can write to the current directory
         snprintf(file, sizeof(file), "%s_%d.mon",
                  (processName ? processName : "bcmcapture"), getpid());
#endif
      }

      ALOGE("********** CAPTURING TO %s ********* \n", file);
      sCaptureArchive = new Archive(file);
      if (sCaptureArchive->Connect())
      {
         sCaptureArchive->Send((uint8_t*)&ident, sizeof(uint32_t), false);
         sCaptureArchive->Send((uint8_t*)&maj, sizeof(uint32_t), false);
         sCaptureArchive->Send((uint8_t*)&min, sizeof(uint32_t), false);
         sCaptureArchive->Send((uint8_t*)&ptrsize, sizeof(uint32_t), false);
      }
      else
         sCaptureStream = false;
   }
   else if (!sCaptureStream)
   {
      // Capture stopped
      if (sCaptureArchive)
         sCaptureArchive->Flush();
   }
}

static void ChangePort(const Packet &packet)
{
   uint32_t port = packet.Item(1).GetUInt32();
   bool capture = false;

   if (packet.NumItems() > 2)
      capture = packet.Item(2).GetBoolean();

   delete remote;
   remote = new Remote(port);
   if (remote->Connect())
   {
      ALOGD("GPU Monitor initialized on port %d\n", port);

      SetCapture(capture, NULL);
   }
   else
      sOrphaned = true;
}

static void SetBottleneckMode(const Packet &packet)
{
   sBottleneckMode = packet.Item(1).GetUInt32();

   Packet dataPacket(eBOTTLENECKSET);
   dataPacket.AddItem(PacketItem(sBottleneckMode));

   if (packet.NumItems() >= 4)
   {
      // These will determine how often we heartbeat in performance mode
      sPerfNumFrames  = packet.Item(2).GetUInt32();
      sPerfNumSeconds = packet.Item(3).GetFloat();

      // Prevent both being zero
      if (sPerfNumFrames == 0 && sPerfNumSeconds == 0.0f)
         sPerfNumFrames = 1;
   }

   if (remote)
      dataPacket.Send(remote);
}

/**
   Search the named set for a given value.

   This search returns true under these conditions:
   i.  The set is empty (empty set matches all).
   ii. The value is in the set.

   The empty set is a special case.  It means that no selection is to be made,
   so match everything.
**/
static bool inVarSet(uint32_t fourcc, uint32_t val)
{
   std::vector<uint32_t> &set(sVarset[fourcc]);
   std::vector<uint32_t>::iterator it;

   if ( set.size() == 0 )
      return true;

   for ( it = set.begin(); it < set.end(); it++ )
      if ( *it == val )
         return true;

   return false;
}

static void SetVarSet(const Packet &packet)
{
   uint32_t num_items = packet.NumItems();
   uint32_t key       = (uint32_t)packet.Item(1).GetUInt32();
   uint32_t i;

   sVarset[key].clear();
   for ( i = 2; i < num_items; i++ )
      sVarset[key].push_back( (uint32_t)packet.Item(i).GetUInt32() );

   Packet dataPacket(eSETVARSET);
   dataPacket.AddItem(PacketItem(key));

   if (remote)
      dataPacket.Send(remote);

   // Cache certain variable results for performance reasons
   if (key == FOURCC('E','V','T','D'))
   {
      sCaptureEvents = !inVarSet(FOURCC('E', 'V', 'T', 'D'), 1);
      if (sCaptureEvents)
      {
         sEventBuffer.reset();

         // Need a timebase so we can create sensible timestamps for the API funcs
         EGLint         uDummy;
         EGLBoolean     bDummy;
         EGLuint64BRCM  timebase;
         EGLuint64BRCM  nowUs;

         sSetEventCollectionBRCM(EGL_ACQUIRE_EVENTS_BRCM);

         nowUs = plGetTimeNowUs();
         sGetEventDataBRCM(0, NULL, &uDummy, &bDummy, &timebase);

         sEventTimebaseOffset = timebase - nowUs;

         sSetEventCollectionBRCM(EGL_START_EVENTS_BRCM);
      }
      else
      {
         sSetEventCollectionBRCM(EGL_STOP_EVENTS_BRCM);
         sSetEventCollectionBRCM(EGL_RELEASE_EVENTS_BRCM);

         sEventBuffer.reset();
      }
   }

   if (key == FOURCC('C','A','P','T'))
      SetCapture(inVarSet(FOURCC('C','A','P','T'), 1), NULL);

   if (key == FOURCC('E','B','M','S'))
      sEventBuffer.setMaxSize(sVarset[key].back());
}

static void ChangeShader(const Packet &packet)
{
   uint32_t context = packet.Item(1).GetUInt32();
   uint32_t progID = packet.Item(2).GetUInt32();
   uint32_t shaderID = packet.Item(3).GetUInt32();
   const char *shaderSrc = packet.Item(4).GetCharPtr();

   bool changedOk = false;
   char linkError[512];
   linkError[0] = '\0';

   if (context != 0 && progID != 0 && shaderID != 0 && shaderSrc != 0)
   {
      EGLContext curCtx = Real(eglGetCurrentContext());
      GLboolean  isShader = Real(glIsShader(shaderID));
      GLboolean  isProgram = Real(glIsProgram(progID));

      if ((EGLContext)(uintptr_t)context == curCtx && isShader && isProgram)
      {
         GLint linkedOK = 0;
         Real(glShaderSource(shaderID, 1, &shaderSrc, NULL));
         Real(glCompileShader(shaderID));
         Real(glLinkProgram(progID));
         Real(glGetProgramiv(progID, GL_LINK_STATUS, &linkedOK));
         if (linkedOK)
         {
            changedOk = true;
         }
         else
         {
            Real(glGetProgramInfoLog(progID, 512, NULL, linkError));
            linkError[511] = '\0';
         }
      }
   }

   Packet dataPacket(eSHADER_CHANGE_ACK);
   dataPacket.AddItem(PacketItem((uint32_t)changedOk));
   dataPacket.AddItem(PacketItem(linkError));

   if (remote)
      dataPacket.Send(remote);
}

static void SetMinimalModeFilters(const Packet &packet)
{
   sMinimalModeCmds.clear();

   for (uint32_t i = 1; i < packet.NumItems(); i++)
      sMinimalModeCmds.insert((eGLCommand)packet.Item(i).GetUInt32());

   // Ensure the essential ones are always in the set
   sMinimalModeCmds.insert(cmd_eglWaitGL);
   sMinimalModeCmds.insert(cmd_eglWaitNative);
   sMinimalModeCmds.insert(cmd_eglSwapBuffers);
   sMinimalModeCmds.insert(cmd_eglMakeCurrent);
   sMinimalModeCmds.insert(cmd_eglGetError);
   sMinimalModeCmds.insert(cmd_glGetError);
   sMinimalModeCmds.insert(cmd_glFinish);
   sMinimalModeCmds.insert(cmd_glBindFramebuffer);
}

#ifdef __mips__
static bool exit_handler_installed;
static jmp_buf exit_handler;
static unsigned int b = 0;

static void segv_handler(int nSig)
{
   if (exit_handler_installed)
      siglongjmp(exit_handler, 1);
}

static bool IsBadReadPtr(const void* lp, unsigned int ucb)
{
   unsigned int i;
   bool ret = false;
   struct sigaction new_action, old_action;

   /* install temporary segv handler for purpose of catching the error */
   new_action.sa_handler = segv_handler;
   sigemptyset (&new_action.sa_mask);
   new_action.sa_flags = SA_RESTART;

   if (sigaction(SIGSEGV, &new_action, &old_action) == -1)
      return false;

   exit_handler_installed = true;
   if (sigsetjmp(exit_handler, 1))
   {
      ret = true;
      goto error0;
   }

   /* test the array */
   for (i = 0; i < ucb; i++)
   {
      /* use a static here to prevent the optimizer removing the loop */
      b += ((unsigned char *)lp)[i];
   }

error0:

   exit_handler_installed = false;

   /* restore previous version */
   sigaction(SIGSEGV, &old_action, NULL);

   return ret;
}

int backtrace_mips32(void **buffer, int size)
   {
   unsigned int *p;
   unsigned int *ra = (unsigned int *) __builtin_return_address(0);
   unsigned int *sp = (unsigned int *) __builtin_frame_address(0);
   size_t ra_offset;
   size_t stack_size = 0;
   int depth;

   if (!((buffer != NULL) && (size > 0)))
      return 0;

   // look for the stack size of backtrace_mips32
   for (p = (unsigned int *)backtrace_mips32; !stack_size; p++)
   {
      if ((*p & 0xffff0000) == 0x27bd0000) // addiu sp,sp
         stack_size = abs((short)(*p & 0xffff));
      else if (*p == 0x03e00008) // jr, ra (got to the end of the function)
         break;
   }

   // oops, didnt find the stack
   if (stack_size)
      {
      // Compute the stack pointer of the caller
      sp = (unsigned int *)((unsigned int)sp + stack_size);

      for (depth = 0; depth < size && ra; depth++)
      {
         unsigned int count_loops = 0;

         buffer[depth] = ra;

         ra_offset = 0;
         stack_size = 0;

         for (p = ra; !ra_offset || !stack_size; p--)
         {
            count_loops++;

            // Don't keep looking for too long
            if (count_loops > 1024 * 64)
               return depth;

            if (IsBadReadPtr(p, 1))
               return depth;

            if (*p == 0x1000ffff)
               return depth;

            switch (*p & 0xffff0000)
            {
               case 0x27bd0000:  // addiu sp,sp
                  stack_size = abs((short)(*p & 0xffff));
                  if (!stack_size)
                     return depth;
                  break;
               case 0xafbf0000:  // sw ra,????
                  ra_offset = (short)(*p & 0xffff);
                  break;
               case 0xffbf0000:
                  ra_offset = (short)(*p & 0xffff);
                  break;
               case 0x3c1c0000:  // lui gp, 0
                  return depth + 1;
               default:
                  break;
            }

            if (ra_offset && IsBadReadPtr((void *)((unsigned int)sp+ra_offset), 1))
               return depth;

            if (ra_offset)
               ra = *(unsigned int **)((unsigned int)sp + ra_offset);

            if (stack_size)
               sp = (unsigned int *)((unsigned int)sp + stack_size);
         }
      }

      return depth;
   }
         else
      return 0;
}
#endif // __mips__

/*
static void print_backtrace(void)
{
#ifdef __mips__
   // do a backtrace, if available
   void *array[255];
   int size;
   int i;

   size = backtrace_mips32(array, sizeof(array));

   for (i = 0; i < size; i++)
   {
      Dl_info dlinfo;
      dladdr(array[i], &dlinfo);
      printf("%s, %s, %p\n",  dlinfo.dli_fname, dlinfo.dli_sname, dlinfo.dli_saddr);
   }
#endif // __mips__
}
*/

static void BackTrace(const Packet &packet)
{
#ifdef __mips__
   Packet dataPacket(eBACKTRACE);

   // do a backtrace, if available
   void *array[512];
   static char result[512][100];
   static char result2[512][100];
   int size;
   int i;
   char* name = 0;

   size = backtrace_mips32(array, sizeof(array));

   dataPacket.AddItem(PacketItem(size));

   for (i = 0; i < size; i++)
   {
      Dl_info dlinfo;
      dladdr(array[i], &dlinfo);

      int status;
      name = __cxxabiv1::__cxa_demangle(dlinfo.dli_sname,0,0, &status);
      if(!name)
          name = (char*)dlinfo.dli_sname;

      sprintf(result[i], "%s(%s)[%p]\n", dlinfo.dli_fname, name, dlinfo.dli_saddr);
      sprintf(result2[i], "%s %p\n", name, dlinfo.dli_saddr);

      if(name != dlinfo.dli_sname)
          free(name);

      dataPacket.AddItem(PacketItem(result[i]));
      dataPacket.AddItem(PacketItem(result2[i]));
   }

   if (remote)
      dataPacket.Send(remote);
#else
   // Backtrace only available in glibc.  Make change in the above when we move
   Packet dataPacket(eBACKTRACE);
   if (remote)
      dataPacket.Send(remote);
#endif // __mips__
}

static bool PostPacketize(Packet *p)
{
   static uint32_t prevMB = 0;

   if (sCaptureStream)
   {
      p->Send(sCaptureArchive);

      uint32_t mb = (uint32_t)(sCaptureArchive->BytesWritten() / (1024 * 1024));
      if (mb > prevMB)
      {
         ALOGD("Capture file = %d MB\n", mb);
         prevMB = mb;
      }

      return true;
   }

   if (remote == NULL)
      return false;

   bool gotDone = false;
   bool skip = false;

   sSendNextRet = false;

   p->Send(remote);

   while (!gotDone)
   {
      Packet retPacket;
      if (!remote->ReceivePacket(&retPacket))
      {
         ALOGE("Didn't receive a return control packet\n");
         sOrphaned = true;
         gotDone = true;
         break;
      }

      if (retPacket.NumItems() > 0)
      {
         Control::eControlAction action = (Control::eControlAction)retPacket.Item(0).GetUInt32();
         switch (action)
         {
         case Control::eDone                  : gotDone = true; break;
         case Control::eSkip                  : gotDone = true; skip = true; break;
         case Control::eDisable               : sEnabled = false; SetMinimalModeFilters(retPacket); gotDone = true; break;
         case Control::eEnable                : sEnabled = true; gotDone = true; break;
         case Control::eGetGLBuffer           : GetGLBuffer(retPacket); break;
         case Control::eGetUniforms           : GetUniforms(retPacket); break;
         case Control::eBottleneck            : SetBottleneckMode(retPacket); break;
         case Control::eGetPerfData           : GetPerfData(retPacket); break;
         case Control::eGetEventData          : GetEventData(retPacket); break;
         case Control::eDisconnect            : sOrphaned = true; gotDone = true; delete remote; remote = NULL; break;
         case Control::eChangePort            : ChangePort(retPacket); gotDone = true; break;
         case Control::eGetState              : GetState(retPacket); break;
         case Control::eGetMemory             : GetMemory(retPacket); break;
         case Control::eSetVarSet             : SetVarSet(retPacket); break;
         case Control::eChangeShader          : ChangeShader(retPacket); break;
         case Control::eBackTrace             : BackTrace(retPacket); break;
         case Control::eGetBufferObjectData   : GetGLBufferObjectData(retPacket); break;
         case Control::eGetPIQData            : GetPIQData(retPacket); break;
         case Control::eGetSyncObjData        : GetSyncObjData(retPacket); break;
         case Control::eGetQueryObjData       : GetQueryObjData(retPacket); break;
         case Control::eGetVertexArrayObjData : GetVertexArrayObjData(retPacket); break;
         case Control::eGetProgramPipelineData: GetProgramPipelineData(retPacket); break;
         case Control::eGetInfoData           : GetInfoData(retPacket); break;
         case Control::eGetFramebufferInfo    : GetFramebufferInfo(retPacket); break;
         case Control::eGetRenderbufferInfo   : GetRenderbufferInfo(retPacket); break;
         }
      }
   }

   return !skip;
}

static uint32_t AttribListSize(const EGLint *attribList)
{
   uint32_t size = 0;

   if (attribList == NULL)
      return 0;

   // Only check the 'name' value as some values can be legally EGL_NONE
   while (*attribList++ != EGL_NONE)
   {
      attribList++; // skip 'value'
      size += (2 * sizeof(EGLint));
   }

   return size + (2 * sizeof(EGLint)); // Include the EGL_NONE token
}

static uint32_t LightParamsSize(GLenum pname)
{
   switch (pname)
   {
   case GL_AMBIENT :
   case GL_DIFFUSE :
   case GL_SPECULAR :
   case GL_POSITION :         return 4;
   case GL_SPOT_DIRECTION :   return 3;
   default :                  return 1;
   }

   return 1;
}

static uint32_t MaterialParamsSize(GLenum pname)
{
   switch (pname)
   {
   case GL_SHININESS :     return 1;
   //case GL_COLOR_INDEXES : return 3;
   default :               return 4;
   }

   return 4;
}

static uint32_t GetESMajorVersion()
{
   EGLint val;
   Real(eglQueryContext)(Real(eglGetCurrentDisplay)(), Real(eglGetCurrentContext)(), EGL_CONTEXT_CLIENT_VERSION, &val);
   return val;
}

static uint32_t BytesForType(GLenum type)
{
   switch (type)
   {
   case GL_BYTE :
   case GL_UNSIGNED_BYTE :  return 1;
   case GL_HALF_FLOAT :
   case GL_SHORT :
   case GL_UNSIGNED_SHORT : return 2;
   case GL_INT :
   case GL_UNSIGNED_INT :
   case GL_FIXED :
   case GL_INT_2_10_10_10_REV :
   case GL_UNSIGNED_INT_2_10_10_10_REV :
   case GL_FLOAT :          return 4;
   }

   ALOGE("Unknown type (%08X) in BytesForType\n", type);
   return 0;
}

extern "C"
{

/*
static bool IsDebugMode()
{
   return sBottleneckMode == Control::eUnset;
}

static bool IsPerfMode()
{
   return sBottleneckMode != Control::eUnset;
}
*/

static void LogGLError(void)
{
   GLenum err = Real(glGetError)();
   if (err != GL_NO_ERROR)
   {
      uint32_t tid = plGetThreadID();

      GLenum curErr = sGLErrors[tid].Get();
      if (curErr == GL_NO_ERROR)
         sGLErrors[tid] = ErrorGL(err);

      // Tell GPUMonitor that we have an error
      Packet   p(eGL_ERROR);
      sCurPacket = &p;
      Packetize((uint32_t)err);
      PostPacketize(&p);
   }
}

static void LogEGLError(void)
{
   uint32_t tid = plGetThreadID();
   EGLint   err = Real(eglGetError)();

   sEGLErrors[tid] = ErrorEGL(err);

   if (err != EGL_SUCCESS)
   {
      // Tell GPUMonitor that we have an error
      Packet   p(eGL_ERROR);
      sCurPacket = &p;
      Packetize((uint32_t)err);
      PostPacketize(&p);
   }
}

static bool WantCommand(eGLCommand cmd)
{
   return !gAPIReentrancy && !sOrphaned && sBottleneckMode == Control::eUnset &&
          (sEnabled || (sMinimalModeCmds.find(cmd) != sMinimalModeCmds.end()));
}

#define AddEventEx(id, type, code, bytes)\
{\
   if (sEventApiTrack != -1 && (code) != -1)\
   {\
      uint32_t *p;\
      p = sEventBuffer.getPointerToWriteData(bytes); \
      if (sEventBuffer.size() == bytes)\
      {\
         sFirstApiCapTime = plGetTimeNowUs() + sEventTimebaseOffset; \
         *((uint64_t*)p) = sFirstApiCapTime; p += 2; \
      }\
      else\
      {\
         *((uint64_t*)p) = plGetTimeNowUs() + sEventTimebaseOffset; p += 2; \
      }\
      *p++ = sEventApiTrack; \
      *p++ = (id); \
      *p++ = (code); \
      *p++ = (type); \
      extraPtr = p;\
   }\
}

#define AddEvent(func, args, id, type)\
{\
   uint32_t *extraPtr = NULL;\
   AddEventEx(id, type, sApiEventCode, 28);\
   if (extraPtr)\
      *extraPtr = cmd_##func;\
}

#define AddTextureEvent(func, args, id, type, target, level)\
{\
   uint32_t *extraPtr = NULL;\
   AddEventEx(id, type, sTextureApiEventCode, 36);\
   if (extraPtr)\
   {\
      GLint texName;\
      switch (target)\
      {\
      case GL_TEXTURE_2D         : Real(glGetIntegerv(GL_TEXTURE_BINDING_2D, &texName)); break;\
      case GL_TEXTURE_2D_ARRAY   : Real(glGetIntegerv(GL_TEXTURE_BINDING_2D_ARRAY, &texName)); break;\
      case GL_TEXTURE_3D         : Real(glGetIntegerv(GL_TEXTURE_BINDING_3D, &texName)); break;\
      default                    : Real(glGetIntegerv(GL_TEXTURE_BINDING_CUBE_MAP, &texName)); break;\
      }\
      *extraPtr++ = cmd_##func; \
      *extraPtr++ = texName;\
      *extraPtr++ = level;\
   }\
}

#define RealTextureApiEvent(func, args, target, level)\
{ \
   if (!sOrphaned)\
   {\
      if (sEventApiTrack == -1 || sApiEventCode == -1)\
      {\
         sEventApiTrack = sGetEventConstantBRCM(EGL_NUM_EVENT_TRACKS_BRCM);\
         sApiEventCode  = sGetEventConstantBRCM(EGL_NUM_EVENTS_BRCM);\
         if (sApiEventCode != -1)\
            sTextureApiEventCode = sApiEventCode + 1;\
      }\
      \
      if (sCaptureEvents)\
         AddTextureEvent(func, args, sEventId, 0, target, level);\
   }\
   \
   Real(func) args; \
   \
   if (!sOrphaned)\
   {\
      if (sCaptureEvents)\
         AddTextureEvent(func, args, sEventId, 1, target, level);\
      sEventId++;\
   }\
}

#define RealApiEvent(func, args)\
{ \
   if (!sOrphaned)\
   {\
      if (sEventApiTrack == -1 || sApiEventCode == -1)\
      {\
         sEventApiTrack = sGetEventConstantBRCM(EGL_NUM_EVENT_TRACKS_BRCM);\
         sApiEventCode  = sGetEventConstantBRCM(EGL_NUM_EVENTS_BRCM);\
         if (sApiEventCode != -1)\
            sTextureApiEventCode = sApiEventCode + 1;\
      }\
      \
      if (!sOrphaned && sCaptureEvents)\
         AddEvent(func, args, sEventId, 0);\
   }\
   \
   Real(func) args; \
   \
   if (!sOrphaned)\
   {\
      if (sCaptureEvents)\
         AddEvent(func, args, sEventId, 1);\
      sEventId++;\
   }\
}

#define RetRealApiEvent(retType, func, args)\
{ \
   if (!sOrphaned)\
   {\
      if (sEventApiTrack == -1 || sApiEventCode == -1)\
      {\
         sEventApiTrack = sGetEventConstantBRCM(EGL_NUM_EVENT_TRACKS_BRCM);\
         sApiEventCode  = sGetEventConstantBRCM(EGL_NUM_EVENTS_BRCM);\
         if (sApiEventCode != -1)\
            sTextureApiEventCode = sApiEventCode + 1;\
      }\
      \
      if (sCaptureEvents)\
         AddEvent(func, args, sEventId, 0);\
   }\
   \
   retType r = Real(func) args; \
   \
   if (!sOrphaned)\
   {\
      if (sCaptureEvents)\
         AddEvent(func, args, sEventId, 1);\
      sEventId++;\
   }\
   return r; \
}

#define RealApiEventRet(varName, retType, func, args)\
   retType varName;\
   if (!sOrphaned)\
   {\
      if (sEventApiTrack == -1 || sApiEventCode == -1)\
      {\
         sEventApiTrack = sGetEventConstantBRCM(EGL_NUM_EVENT_TRACKS_BRCM);\
         sApiEventCode  = sGetEventConstantBRCM(EGL_NUM_EVENTS_BRCM);\
      }\
      \
      if (sCaptureEvents)\
         AddEvent(func, args, sEventId, 0);\
   }\
   \
   varName = Real(func) args; \
   \
   if (!sOrphaned)\
   {\
      if (sCaptureEvents)\
         AddEvent(func, args, sEventId, 1);\
      sEventId++;\
   }

#define Func(api, func, args) \
{ \
   APIInitAndLock __locker(PL_FUNCTION);\
   if (WantCommand(cmd_##func))\
   {\
      Packet   __p(eAPI_FUNCTION); \
      sCurPacket = &__p; \
      __p.AddItem(cmd_##func); \
      Packetize args; \
      bool ret = PostPacketize(&__p);\
      if (ret) \
      {\
         TIMESTAMP __start, __end; \
         plGetTime(&__start); \
         Real(func) args; \
         plGetTime(&__end);   \
         Log##api##Error(); \
         RetVoidFunc(plTimeDiffNano(&__start, &__end)); \
      }\
   }\
   else\
      RealApiEvent(func, args); \
}

static void PacketizeGenData(GLsizei count, GLuint *data)
{
   for (GLsizei i = 0; i < count; i++)
      Packetize(data[i]);
}

#define GenFunc(api, func, args) \
{ \
   APIInitAndLock locker(PL_FUNCTION);\
   if (WantCommand(cmd_##func))\
   {\
      Packet   __p(eAPI_FUNCTION); \
      sCurPacket = &__p; \
      __p.AddItem(cmd_##func); \
      Packetize args; \
      bool ret = PostPacketize(&__p);\
      if (ret) \
      {\
         TIMESTAMP __start, __end; \
         plGetTime(&__start); \
         Real(func) args; \
         plGetTime(&__end);   \
         Log##api##Error(); \
         Packet   __p(eRET_CODE); \
         sCurPacket = &__p; \
         __p.AddItem(2); \
         __p.AddItem(plTimeDiffNano(&__start, &__end)); \
         PacketizeGenData args; \
         PostPacketize(&__p); \
      }\
   }\
   else\
      RealApiEvent(func, args); \
}

#define DrawFunc(api, func, args) \
{ \
   APIInitAndLock locker(PL_FUNCTION);\
   if (WantCommand(cmd_##func))\
   {\
      Packet   __p(eAPI_FUNCTION); \
      sCurPacket = &__p; \
      __p.AddItem(cmd_##func); \
      Packetize args; \
      bool ret = PostPacketize(&__p);\
      if (ret) \
      {\
         TIMESTAMP __start, __end; \
         plGetTime(&__start); \
         Real(func) args; \
         plGetTime(&__end);   \
         Log##api##Error(); \
         RetVoidFunc(plTimeDiffNano(&__start, &__end)); \
      }\
   }\
   else if (!sOrphaned && sEnabled && sBottleneckMode == Control::eNullDrawCalls)\
   {\
   }\
   else\
      RealApiEvent(func, args); \
}

#define FuncRet(api, retType, func, args) \
{ \
   APIInitAndLock locker(PL_FUNCTION);\
   if (WantCommand(cmd_##func))\
   {\
      Packet   __p(eAPI_FUNCTION); \
      sCurPacket = &__p; \
      __p.AddItem(cmd_##func); \
      Packetize args; \
      bool ret = PostPacketize(&__p);\
      if (ret) \
      {\
         TIMESTAMP __start, __end; \
         plGetTime(&__start); \
         retType __r = Real(func) args; \
         plGetTime(&__end);   \
         Log##api##Error(); \
         RetFunc(plTimeDiffNano(&__start, &__end), (__r)); \
         return __r; \
      }\
      else \
         return 0; \
   }\
   else \
      RetRealApiEvent(retType, func, args);\
}

#define FuncRetSurface(api, retType, func, args) \
{ \
   APIInitAndLock locker(PL_FUNCTION);\
   if (WantCommand(cmd_##func))\
   {\
      Packet   __p(eAPI_FUNCTION); \
      sCurPacket = &__p; \
      __p.AddItem(cmd_##func); \
      Packetize args; \
      bool ret = PostPacketize(&__p);\
      if (ret) \
      {\
         TIMESTAMP __start, __end; \
         plGetTime(&__start); \
         retType __r = Real(func) args; \
         plGetTime(&__end);   \
         Log##api##Error(); \
         return_EGLSurface(__r, plTimeDiffNano(&__start, &__end)); \
         return __r; \
      }\
      else \
         return 0; \
   }\
   else \
      RetRealApiEvent(retType, func, args);\
}

#define FuncExtra1(api, func, args, item1) \
{ \
   APIInitAndLock locker(PL_FUNCTION);\
   if (WantCommand(cmd_##func))\
   {\
      Packet   __p(eAPI_FUNCTION); \
      sCurPacket = &__p; \
      __p.AddItem(cmd_##func); \
      Packetize args; \
      if (sCaptureStream) \
         sCurPacket->AddItem(PacketItem item1); \
      bool ret = PostPacketize(&__p);\
      if (ret) \
      {\
         TIMESTAMP __start, __end; \
         plGetTime(&__start); \
         Real(func) args; \
         plGetTime(&__end);   \
         Log##api##Error(); \
         RetVoidFunc(plTimeDiffNano(&__start, &__end)); \
      }\
   }\
   else \
      RealApiEvent(func, args); \
}

#define DrawFuncExtra1(api, func, args, item1) \
{ \
   APIInitAndLock locker(PL_FUNCTION);\
   if (WantCommand(cmd_##func))\
   {\
      Packet   __p(eAPI_FUNCTION); \
      sCurPacket = &__p; \
      __p.AddItem(cmd_##func); \
      Packetize args; \
      if (sCaptureStream) \
         sCurPacket->AddItem(PacketItem item1); \
      bool ret = PostPacketize(&__p);\
      if (ret) \
      {\
         TIMESTAMP __start, __end; \
         plGetTime(&__start); \
         Real(func) args; \
         plGetTime(&__end);   \
         Log##api##Error(); \
         RetVoidFunc(plTimeDiffNano(&__start, &__end)); \
      }\
   }\
   else if (!sOrphaned && sBottleneckMode == Control::eNullDrawCalls && \
            (sEnabled || sMinimalModeCmds.find(cmd_##func) != sMinimalModeCmds.end()))\
   {\
   }\
   else \
      RealApiEvent(func, args); \
}

static void TinyTexImage2D(GLenum target, GLint level)
{
   static uint8_t tiny[] = { 180, 80, 80, 255 };
   if (level == 0)
      Real(glTexImage2D)(target, level, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, tiny);
}

static void TinyTexSubImage2D(GLenum target, GLint level)
{
   static uint8_t tiny[] = { 180, 80, 80, 255 };
   if (level == 0)
      Real(glTexSubImage2D)(target, level, 0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, tiny);
}

static void TinyTexImage3D(GLenum target, GLint level)
{
   static uint8_t tiny[] = { 180, 80, 80, 255 };
   if (level == 0)
      Real(glTexImage3D)(target, level, GL_RGBA, 1, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, tiny);
}

static void TinyTexSubImage3D(GLenum target, GLint level)
{
   static uint8_t tiny[] = { 180, 80, 80, 255 };
   if (level == 0)
      Real(glTexSubImage3D)(target, level, 0, 0, 0, 1, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, tiny);
}

static void PacketizePBOData(GLenum target, void *off, GLint len)
{
   GLint mapped, flags, offset, length;
   void *bufPtr;

   // Need to preserve any current mapping
   Real(glGetBufferParameteriv)(target, GL_BUFFER_MAPPED, &mapped);

   if (mapped)
   {
      Real(glGetBufferParameteriv)(target, GL_BUFFER_MAPPED, &mapped);
      Real(glGetBufferParameteriv)(target, GL_BUFFER_ACCESS_FLAGS, &flags);
      Real(glGetBufferParameteriv)(target, GL_BUFFER_MAP_OFFSET, &offset);
      Real(glGetBufferParameteriv)(target, GL_BUFFER_MAP_LENGTH, &length);

      Real(glUnmapBuffer)(target);
   }

   bufPtr = Real(glMapBufferRange)(target, (GLintptr)off, len, GL_MAP_READ_BIT);
   sCurPacket->AddItem(PacketItem(bufPtr, len));
   Real(glUnmapBuffer)(target);

   if (mapped)
      Real(glMapBufferRange)(target, offset, length, flags);
}

#define TextureFuncExtra1MaybePBO(api, func, args, ptr, len, target, level, tinyfunc) \
{ \
   APIInitAndLock locker(PL_FUNCTION);\
   if (WantCommand(cmd_##func))\
   {\
      Packet   p(eAPI_FUNCTION); \
      GLint buf = 0;\
      sCurPacket = &p; \
      p.AddItem(cmd_##func); \
      Packetize args; \
      if (GetESMajorVersion() > 1) \
         Real(glGetIntegerv)(GL_PIXEL_UNPACK_BUFFER_BINDING, &buf);\
      if (sCaptureStream) \
      {\
         if (buf == 0)\
            sCurPacket->AddItem(PacketItem((void*)ptr, len)); \
         else\
         {\
            /* To maintain backwards compatibility */ \
            sCurPacket->AddItem(PacketItem((void*)NULL, 0)); \
            Packetize(ptr);\
         }\
      }\
      else\
      {\
         Packetize(buf != 0);\
         if (buf != 0)\
            PacketizePBOData(GL_PIXEL_UNPACK_BUFFER, (void*)ptr, len);\
      }\
      \
      bool ret = PostPacketize(&p);\
      if (ret) \
      {\
         TIMESTAMP start, end; \
         plGetTime(&start); \
         Real(func) args; \
         plGetTime(&end);   \
         Log##api##Error(); \
         RetVoidFunc(plTimeDiffNano(&start, &end)); \
      }\
   }\
   else if (!sOrphaned && sBottleneckMode == Control::eTinyTextures && \
            (sEnabled || sMinimalModeCmds.find(cmd_##func) != sMinimalModeCmds.end()))\
   {\
      tinyfunc(target, level);\
   }\
   else \
      RealTextureApiEvent(func, args, target, level); \
}

#define FuncExtra1Ret(api, retType, func, args, item1) \
{ \
   APIInitAndLock locker(PL_FUNCTION);\
   if (WantCommand(cmd_##func))\
   {\
      Packet   p(eAPI_FUNCTION); \
      sCurPacket = &p; \
      p.AddItem(cmd_##func); \
      Packetize args; \
      if (sCaptureStream) \
         sCurPacket->AddItem(PacketItem item1); \
      bool ret = PostPacketize(&p);\
      if (ret) \
      {\
         TIMESTAMP start, end; \
         plGetTime(&start); \
         retType r = Real(func) args; \
         plGetTime(&end);   \
         Log##api##Error(); \
         RetFunc(plTimeDiffNano(&start, &end), (r)); \
         return r; \
      }\
      else \
         return 0; \
   }\
   else \
      RetRealApiEvent(retType, func, args);\
}

#define FuncExtra1RetSurface(api, retType, func, args, item1) \
{ \
   APIInitAndLock locker(PL_FUNCTION);\
   if (WantCommand(cmd_##func))\
   {\
      Packet   p(eAPI_FUNCTION); \
      sCurPacket = &p; \
      p.AddItem(cmd_##func); \
      Packetize args; \
      if (sCaptureStream) \
         sCurPacket->AddItem(PacketItem item1); \
      bool ret = PostPacketize(&p);\
      if (ret) \
      {\
         TIMESTAMP start, end; \
         plGetTime(&start); \
         retType r = Real(func) args; \
         plGetTime(&end);   \
         Log##api##Error(); \
         return_EGLSurface(r, plTimeDiffNano(&start, &end)); \
         return r; \
      }\
      else \
         return 0; \
   }\
   else \
      RetRealApiEvent(retType, func, args);\
}

#define FuncExtra2Ret(api, retType, func, args, item1, item2) \
{ \
   APIInitAndLock locker(PL_FUNCTION);\
   if (WantCommand(cmd_##func))\
   {\
      Packet   p(eAPI_FUNCTION); \
      sCurPacket = &p; \
      p.AddItem(cmd_##func); \
      Packetize args; \
      if (sCaptureStream) \
      {\
         sCurPacket->AddItem(PacketItem item1); \
         sCurPacket->AddItem(PacketItem item2); \
      }\
      bool ret = PostPacketize(&p);\
      if (ret) \
      {\
         TIMESTAMP start, end; \
         plGetTime(&start); \
         retType r = Real(func) args; \
         plGetTime(&end);   \
         Log##api##Error(); \
         RetFunc(plTimeDiffNano(&start, &end), (r)); \
         return r; \
      }\
      else \
         return 0; \
   }\
   else \
      RetRealApiEvent(retType, func, args);\
}

#define DebugModeOnlyFunc(api, func, args) \
{ \
   APIInitAndLock locker(PL_FUNCTION);\
   if (!sOrphaned && sBottleneckMode == Control::eUnset)\
   {\
      Packet   p(eAPI_FUNCTION); \
      sCurPacket = &p; \
      p.AddItem(cmd_##func); \
      Packetize args; \
      bool ret = PostPacketize(&p);\
      sSendNextRet = true;\
      if (ret) \
      {\
         TIMESTAMP start, end; \
         plGetTime(&start); \
         Real(func) args; \
         plGetTime(&end);   \
         Log##api##Error(); \
         RetVoidFunc(plTimeDiffNano(&start, &end));\
      }\
   }\
   else \
      RealApiEvent(func, args); \
}

#define DebugModeOnlyFuncRet(api, retType, func, args) \
{ \
   APIInitAndLock locker(PL_FUNCTION);\
   if (!sOrphaned && sBottleneckMode == Control::eUnset)\
   {\
      Packet   p(eAPI_FUNCTION); \
      sCurPacket = &p; \
      p.AddItem(cmd_##func); \
      Packetize args; \
      bool ret = PostPacketize(&p);\
      sSendNextRet = true;\
      if (ret) \
      {\
         TIMESTAMP start, end; \
         plGetTime(&start); \
         retType r = Real(func) args; \
         plGetTime(&end);   \
         Log##api##Error(); \
         RetFunc(plTimeDiffNano(&start, &end), (r)); \
         return r; \
      }\
      else \
         return 0; \
   }\
   else \
      RetRealApiEvent(retType, func, args);\
}

///////////////////////////////////////////////////////
// Functions to hand real GL call returns to GPUMonitor
// Must be called with a GLOBAL_LOCK in place
///////////////////////////////////////////////////////
#define RetFunc(ns, args) \
{ \
   Packet   p(eRET_CODE); \
   sCurPacket = &p; \
   p.AddItem(0); \
   Packetize args; \
   p.AddItem(ns); \
   PostPacketize(&p);\
}

#define RetEnumFunc(ns, args) \
{ \
   Packet   p(eRET_CODE); \
   sCurPacket = &p; \
   p.AddItem(1); \
   Packetize args; \
   p.AddItem(ns); \
   PostPacketize(&p);\
}

#define RetVoidFunc(ns) \
{ \
   Packet   p(eRET_CODE); \
   p.AddItem(2); \
   p.AddItem(ns); \
   PostPacketize(&p);\
}

static EGLConfig ConfigIDToConfig(EGLDisplay disp, EGLint configID)
{
   EGLint      attribs[3];
   EGLConfig   config;
   EGLint      numConfigs;

   attribs[0] = EGL_CONFIG_ID;
   attribs[1] = configID;
   attribs[2] = EGL_NONE;

   Real(eglChooseConfig(disp, attribs, &config, 1, &numConfigs));

   return config;
}

static void return_EGLSurface(EGLSurface s, uint32_t ns)
{
   Packet   p(eRET_CODE);
   sCurPacket = &p;
   p.AddItem(0);
   Packetize(s);

   // Since this is a surface, we'll attach the dimensions etc as extra data
   EGLDisplay disp = Real(eglGetDisplay)(EGL_DEFAULT_DISPLAY);
   EGLint w, h, configId, r, g, b, a, depth, stencil, samples;

   Real(eglQuerySurface)(disp, s, EGL_WIDTH, &w);
   Real(eglQuerySurface)(disp, s, EGL_HEIGHT, &h);
   Real(eglQuerySurface)(disp, s, EGL_CONFIG_ID, &configId);

   // Convert EGL_CONFIG_ID into EGLConfig
   EGLConfig config = ConfigIDToConfig(disp, configId);

   // NOTE: R, G, B & A MUST come first to preserve backwards compatibility
   Real(eglGetConfigAttrib)(disp, config, EGL_RED_SIZE, &r);
   Real(eglGetConfigAttrib)(disp, config, EGL_GREEN_SIZE, &g);
   Real(eglGetConfigAttrib)(disp, config, EGL_BLUE_SIZE, &b);
   Real(eglGetConfigAttrib)(disp, config, EGL_ALPHA_SIZE, &a);
   Real(eglGetConfigAttrib)(disp, config, EGL_DEPTH_SIZE, &depth);
   Real(eglGetConfigAttrib)(disp, config, EGL_STENCIL_SIZE, &stencil);
   Real(eglGetConfigAttrib)(disp, config, EGL_SAMPLES, &samples);

   Packetize(w, h, r, g, b, a, depth, stencil, samples);

   p.AddItem(ns);

   PostPacketize(&p);
}

#ifdef WIN32
static bool determine_capture_params(char *procname, char *capname, size_t buflen)
{
   if (buflen > 0)
   {
      *procname = '\0';
      *capname = '\0';
   }
   return false;
}
#endif

#ifdef ANDROID
static bool determine_capture_params(char *procname, char *capname, size_t buflen)
{
   // Get our process name from the command line
   char cmdfile[32], cmdline[1024];

   snprintf(cmdfile, sizeof(cmdfile), "/proc/%d/cmdline", getpid());

   if (buflen > 0)
      procname[0] = '\0';

   FILE *fp = fopen(cmdfile, "r");
   if (fp != NULL)
   {
      fgets(cmdline, sizeof(cmdline), fp);
      fclose(fp);

      strncpy(procname, strtok(cmdline, ": \n"), buflen);
   }

   // Examine props
   char val[PROPERTY_VALUE_MAX];
   property_get("debug.egl.hw.gpumon.noterm", val, "");
   if (strlen(val) > 0)
      sIgnoreEGLTerminate = true;
   else
      sIgnoreEGLTerminate = false;

   property_get("debug.egl.hw.gpumon.appname", val, "");
   strncpy(capname, val, buflen);

   property_get("debug.egl.hw.gpumon.ip", val, "");
   if (strlen(val) > 0)
      return false;

   property_get("debug.egl.hw.gpumon.capture", val, "");
   if (strlen(val) == 0 || atoi(val) == 0)
      return false;

   return true;
}
#endif

#if !defined(WIN32) && !defined(ANDROID)
static bool determine_capture_params(char *procname, char *capname, size_t buflen)
{
   // Get our process name from the command line
   char cmdfile[32], cmdline[1024];

   snprintf(cmdfile, sizeof(cmdfile), "/proc/%d/comm", getpid());

   if (buflen > 0)
   {
      procname[0] = '\0';
      *capname = '\0';
   }

   FILE *fp = fopen(cmdfile, "r");
   if (fp != NULL)
   {
      fgets(cmdline, sizeof(cmdline), fp);
      fclose(fp);

      strncpy(procname, strtok(cmdline, ": \n"), buflen);
   }

   char *value = getenv("GPUMonitorCapture");
   if (value == NULL || atoi(value) == 0)
      return false;

   return true;
}
#endif

#define NAME_SIZE 1024

// Must be called with GLOBAL_LOCK in place
static void gpumon_initialize()
{
   static bool alreadyInited = false;
   static bool alreadyLoaded = false;

   if (alreadyInited)
   {
      if (!alreadyLoaded)
      {
         if (fill_real_func_table(&sRealFuncs))
            alreadyLoaded = true;
      }
      return;
   }

   alreadyInited = true;

   memset(&sRealFuncs, 0, sizeof(REAL_GL_API_TABLE));

   // Fill the table
   if (!alreadyLoaded)
   {
      if (!fill_real_func_table(&sRealFuncs))
      {
         ALOGE("GPUMonitor disabled\n");
         sOrphaned = true;
         return;
      }
      alreadyLoaded = true;
   }

   // Initialise perf counter and event monitor extensions
   InitExtensions();

   sMinimalModeCmds.insert(cmd_eglWaitGL);
   sMinimalModeCmds.insert(cmd_eglWaitNative);
   sMinimalModeCmds.insert(cmd_eglSwapBuffers);
   sMinimalModeCmds.insert(cmd_glFinish);

   char ourProcName[NAME_SIZE];
   char exeToCapture[NAME_SIZE];

   // Do we have an environment / property that tells us to capture?
   bool capture = determine_capture_params(ourProcName, exeToCapture, NAME_SIZE);

   ALOGD("*************************** GPUMON : capture = %d, process = %s, exeToCapture = %s\n", capture ? 1 : 0, ourProcName, exeToCapture);

   if (capture)
   {
      // If no exe was specified, assume this one
      if (strlen(exeToCapture) == 0)
         strcpy(exeToCapture, ourProcName);

      if (strcmp(exeToCapture, ourProcName))
      {
         // This app does not match what we want to capture
         capture = false;
         sOrphaned = true;
         return;
      }
   }

   SetCapture(capture, ourProcName);

   // Catch ctrl-c so we can ensure the capture buffers are flushed and remote is disconnected
   signal(SIGINT, CtrlCHandler);

   if (!sCaptureStream)
   {
      ALOGD("Waiting for GPU Monitor connection...\n");

      remote = new Remote(28015);
      if (remote->Connect())
      {
         ALOGD("GPU Monitor initializing...\n");

         // Send our init packet
         Packet   p(eINIT);
         sCurPacket = &p;

#ifdef NDEBUG
         static const bool debug = false;
#else
         static const bool debug = true;
#endif
         static const uint32_t ptrSize = sizeof(void*);

         Packetize((uint32_t)getpid(), debug, SPYHOOK_MAJOR_VER, SPYHOOK_MINOR_VER, GLESVER, ptrSize);
         PostPacketize(&p);
         plGetTimeNowMs(); // Initialize the internal timebases
         plGetTimeNowUs();
      }
      else
      {
         sOrphaned = true;
         return;
      }
   }
}

/******************************************************************************/
static void send_thread_change(uint32_t threadID, EGLContext context)
{
   if (!sOrphaned && sBottleneckMode == Control::eUnset)
   {
      Packet   p(eTHREAD_CHANGE);
      sCurPacket = &p;
      Packetize(threadID, context, Real(eglGetCurrentSurface(EGL_DRAW)), Real(eglGetCurrentSurface(EGL_READ)));
      PostPacketize(&p);
   }
}

DLLEXPORT GLenum DLLEXPORTENTRY specialized_glGetError (void)
{
   APIInitAndLock locker(PL_FUNCTION);
   if (WantCommand(cmd_glGetError))
   {
      Packet   p(eAPI_FUNCTION);
      sCurPacket = &p;
      p.AddItem(cmd_glGetError);
      Packetize();
      bool ret = PostPacketize(&p);
      sSendNextRet = true;
      if (ret)
      {
         uint32_t tid = plGetThreadID();

         TIMESTAMP start, end;
         plGetTime(&start);
         GLenum err = sGLErrors[tid].Get(); // Get the first stored error
         sGLErrors[tid] = ErrorGL();        // Clear the stored error
         // Make a call to get timing data, we know there won't be an error
         Real(glGetError)();
         plGetTime(&end);
         RetFunc(plTimeDiffNano(&start, &end), (err));
         return err;
      }
      else
         return 0;
   }
   else
      RetRealApiEvent(GLenum, glGetError, ());
}

DLLEXPORT void DLLEXPORTENTRY specialized_glBindFramebuffer (GLenum target, GLuint framebuffer)
{
   APIInitAndLock locker(PL_FUNCTION);
   if (WantCommand(cmd_glBindFramebuffer))
   {
      Packet   p(eAPI_FUNCTION);
      sCurPacket = &p;
      p.AddItem(cmd_glBindFramebuffer);
      Packetize(target, framebuffer);
      bool ret = PostPacketize(&p);
      sSendNextRet = true;
      if (ret)
      {
         TIMESTAMP start, end;
         plGetTime(&start);
         Real(glBindFramebuffer)(target, framebuffer);
         plGetTime(&end);
         LogGLError();
         RetVoidFunc(plTimeDiffNano(&start, &end));
      }
   }
   else if (!sOrphaned && sBottleneckMode == Control::eTinyViewport)
   {
      Real(glBindFramebuffer)(target, framebuffer);
      Real(glViewport(0, 0, 1, 1));
   }
   else
      RealApiEvent(glBindFramebuffer, (target, framebuffer));
}

DLLEXPORT void DLLEXPORTENTRY specialized_glBlendEquation (GLenum mode)
{
   if (!sOrphaned && sBottleneckMode == Control::eOverdraw)
      mode = GL_FUNC_ADD;
   Func(GL, glBlendEquation, (mode))
}

DLLEXPORT void DLLEXPORTENTRY specialized_glBlendEquationSeparate (GLenum modeRGB, GLenum modeAlpha)
{
   if (!sOrphaned && sBottleneckMode == Control::eOverdraw)
      modeRGB = modeAlpha = GL_FUNC_ADD;
   Func(GL, glBlendEquationSeparate, (modeRGB, modeAlpha))
}

DLLEXPORT void DLLEXPORTENTRY specialized_glBlendFunc (GLenum sfactor, GLenum dfactor)
{
   if (!sOrphaned && sBottleneckMode == Control::eOverdraw)
      sfactor = dfactor = GL_ONE;

   Func(GL, glBlendFunc, (sfactor, dfactor))
}
DLLEXPORT void DLLEXPORTENTRY specialized_glBlendFuncSeparate (GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha)
{
   if (!sOrphaned && sBottleneckMode == Control::eOverdraw)
   {
      srcRGB = dstRGB = srcAlpha = GL_ONE;
      dstAlpha = GL_ZERO;
   }

   Func(GL, glBlendFuncSeparate, (srcRGB, dstRGB, srcAlpha, dstAlpha))
}
DLLEXPORT void DLLEXPORTENTRY specialized_glBufferData (GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage)
{
   APIInitAndLock locker(PL_FUNCTION);
   if (WantCommand(cmd_glBufferData))
   {
      Packet   p(eAPI_FUNCTION);
      sCurPacket = &p;
      p.AddItem(cmd_glBufferData);
      Packetize(target, size, data, usage);
      if (sCaptureStream)
         sCurPacket->AddItem(PacketItem((void*)data, size));
      bool ret = PostPacketize(&p);

      if (sCaptureStream)
      {
         // For index buffers, we need to store a record of the max index
         if (target == GL_ELEMENT_ARRAY_BUFFER && data != NULL)
         {
            EGLContext curCtx = Real(eglGetCurrentContext)();
            GLint      elementBind = 0;
            GLuint     maxByteIndx = 0;
            GLuint     maxShortIndx = 0;
            GLuint     maxIntIndx = 0;
            GLuint     nhByteIndx = 0;
            GLuint     nhShortIndx = 0;
            GLuint     nhIntIndx = 0;

            Real(glGetIntegerv)(GL_ELEMENT_ARRAY_BUFFER_BINDING, &elementBind);

            for (GLint i = 0; i < size; i++)
            {
               if (((GLubyte*)data)[i] > maxByteIndx)
               {
                  nhByteIndx = maxByteIndx;
                  maxByteIndx = ((GLubyte*)data)[i];
               }

               if ((i & 1) == 0)
               {
                  if (((GLushort*)data)[i/2] > maxShortIndx)
                  {
                     nhShortIndx = maxShortIndx;
                     maxShortIndx = ((GLushort*)data)[i/2];
                  }
               }

               if ((i % 4) == 0)
               {
                  if (((GLuint*)data)[i/4] > maxIntIndx)
                  {
                     nhIntIndx = maxIntIndx;
                     maxIntIndx = ((GLuint*)data)[i/4];
                  }
               }
            }

            Buffer *buffer = &sContexts[(uintptr_t)curCtx].m_buffers[elementBind];
            buffer->m_maxByteIndx = maxByteIndx;
            buffer->m_maxShortIndx = maxShortIndx;
            buffer->m_maxIntIndx = maxIntIndx;
            buffer->m_nextHighestByteIndx = nhByteIndx;
            buffer->m_nextHighestShortIndx = nhShortIndx;
            buffer->m_nextHighestIntIndx = nhIntIndx;
         }
      }

      if (ret)
      {
         TIMESTAMP start, end;
         plGetTime(&start);
         Real(glBufferData)(target, size, data, usage);
         plGetTime(&end);
         LogGLError();
         RetVoidFunc(plTimeDiffNano(&start, &end));
      }
   }
   else
      RealApiEvent(glBufferData, (target, size, data, usage));
}
DLLEXPORT void DLLEXPORTENTRY specialized_glBufferSubData (GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid *data)
{
   APIInitAndLock locker(PL_FUNCTION);
   if (WantCommand(cmd_glBufferSubData))
   {
      Packet   p(eAPI_FUNCTION);
      sCurPacket = &p;
      p.AddItem(cmd_glBufferSubData);
      Packetize(target, offset, size, data);
      if (sCaptureStream)
         sCurPacket->AddItem(PacketItem((void*)data, size));
      bool ret = PostPacketize(&p);

      if (sCaptureStream)
      {
         // Update max indices for indexed buffers
         if (target == GL_ELEMENT_ARRAY_BUFFER && data != NULL)
         {
            EGLContext curCtx = Real(eglGetCurrentContext)();
            GLint      elementBind = 0;
            GLuint     maxByteIndx = 0;
            GLuint     maxShortIndx = 0;
            GLuint     maxIntIndx = 0;
            GLuint     nhByteIndx = 0;
            GLuint     nhShortIndx = 0;
            GLuint     nhIntIndx = 0;

            Real(glGetIntegerv)(GL_ELEMENT_ARRAY_BUFFER_BINDING, &elementBind);

            for (GLint i = 0; i < size; i++)
            {
               if (((GLubyte*)data)[i] > maxByteIndx)
               {
                  nhByteIndx = maxByteIndx;
                  maxByteIndx = ((GLubyte*)data)[i];
               }

               if ((i & 1) == 0)
               {
                  if (((GLushort*)data)[i/2] > maxShortIndx)
                  {
                     nhShortIndx = maxShortIndx;
                     maxShortIndx = ((GLushort*)data)[i/2];
                  }
               }

               if ((i % 4) == 0)
               {
                  if (((GLuint*)data)[i/4] > maxIntIndx)
                  {
                     nhIntIndx = maxIntIndx;
                     maxIntIndx = ((GLuint*)data)[i/4];
                  }
               }
            }

            Buffer *buffer = &sContexts[(uintptr_t)curCtx].m_buffers[elementBind];
            if (maxByteIndx > buffer->m_maxByteIndx)
            {
               buffer->m_maxByteIndx = maxByteIndx;
               buffer->m_nextHighestByteIndx = nhByteIndx;
            }
            if (maxShortIndx > buffer->m_maxShortIndx)
            {
               buffer->m_maxShortIndx = maxShortIndx;
               buffer->m_nextHighestShortIndx = nhShortIndx;
            }
            if (maxIntIndx > buffer->m_maxIntIndx)
            {
               buffer->m_maxIntIndx = maxIntIndx;
               buffer->m_nextHighestIntIndx = nhIntIndx;
            }
         }
      }

      if (ret)
      {
         TIMESTAMP start, end;
         plGetTime(&start);
         Real(glBufferSubData)(target, offset, size, data);
         plGetTime(&end);
         LogGLError();
         RetVoidFunc(plTimeDiffNano(&start, &end));
      }
   }
   else
      RealApiEvent(glBufferSubData, (target, offset, size, data));
}
DLLEXPORT void DLLEXPORTENTRY specialized_glClearColor (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
   if (!sOrphaned && sBottleneckMode == Control::eOverdraw)
   {
      red = green = blue = 0.0f;
      alpha = 1.0f;
   }
   Func(GL, glClearColor, (red, green, blue, alpha))
}
DLLEXPORT void DLLEXPORTENTRY specialized_glClipPlanef (GLenum plane, const GLfloat *equation)
   FuncExtra1(GL, glClipPlanef, (plane, equation), ((void*)equation, 4 * sizeof(GLfloat)))
DLLEXPORT void DLLEXPORTENTRY specialized_glClipPlanex (GLenum plane, const GLfixed *equation)
   FuncExtra1(GL, glClipPlanex, (plane, equation), ((void*)equation, 4 * sizeof(GLfixed)))
DLLEXPORT void DLLEXPORTENTRY specialized_glColorPointer (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
   APIInitAndLock locker(PL_FUNCTION);
   if (WantCommand(cmd_glColorPointer))
   {
      Packet   p(eAPI_FUNCTION);
      sCurPacket = &p;
      p.AddItem(cmd_glColorPointer);
      Packetize(size, type, stride, pointer);

      if (sCaptureStream)
      {
         GLint v[4];
         Real(glGetIntegerv)(GL_ARRAY_BUFFER_BINDING, v);
         Packetize(v[0] != 0);
      }

      bool ret = PostPacketize(&p);
      if (ret)
      {
         TIMESTAMP start, end;
         plGetTime(&start);
         Real(glColorPointer)(size, type, stride, pointer);
         plGetTime(&end);
         LogGLError();
         RetVoidFunc(plTimeDiffNano(&start, &end));
      }
   }
   else
      RealApiEvent(glColorPointer, (size, type, stride, pointer));
}
DLLEXPORT void DLLEXPORTENTRY specialized_glCompressedTexImage2D (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data)
   TextureFuncExtra1MaybePBO(GL, glCompressedTexImage2D, (target, level, internalformat, width, height, border, imageSize, data),
                      data, imageSize, target, level, TinyTexImage2D)
DLLEXPORT void DLLEXPORTENTRY specialized_glCompressedTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data)
   TextureFuncExtra1MaybePBO(GL, glCompressedTexSubImage2D, (target, level, xoffset, yoffset, width, height, format, imageSize, data),
                      data, imageSize, target, level, TinyTexSubImage2D)
DLLEXPORT void DLLEXPORTENTRY specialized_glDeleteBuffers (GLsizei n, const GLuint *buffers)
   FuncExtra1(GL, glDeleteBuffers, (n, buffers), ((void*)buffers, n * sizeof(GLuint)))
DLLEXPORT void DLLEXPORTENTRY specialized_glDeleteFramebuffers (GLsizei n, const GLuint* framebuffers)
   FuncExtra1(GL, glDeleteFramebuffers, (n, framebuffers), ((void*)framebuffers, n * sizeof(GLuint)))
DLLEXPORT void DLLEXPORTENTRY specialized_glDeleteRenderbuffers (GLsizei n, const GLuint* renderbuffers)
   FuncExtra1(GL, glDeleteRenderbuffers, (n, renderbuffers), ((void*)renderbuffers, n * sizeof(GLuint)))
DLLEXPORT void DLLEXPORTENTRY specialized_glDeleteTextures (GLsizei n, const GLuint* textures)
   FuncExtra1(GL, glDeleteTextures, (n, textures), ((void*)textures, n * sizeof(GLuint)))
DLLEXPORT void DLLEXPORTENTRY specialized_glDisable (GLenum cap)
{
   if (cap == GL_BLEND && !sOrphaned && sBottleneckMode == Control::eOverdraw)
      return;

   Func(GL, glDisable, (cap))
}
DLLEXPORT void DLLEXPORTENTRY specialized_glDrawArrays (GLenum mode, GLint first, GLsizei count)
{
   APIInitAndLock locker(PL_FUNCTION);
   if (WantCommand(cmd_glDrawArrays))
   {
      Packet   p(eAPI_FUNCTION);
      sCurPacket = &p;
      p.AddItem(cmd_glDrawArrays);
      Packetize(mode, first, count);

      if (sCaptureStream)
      {
         {
            GLint type, bytesPerVert, stride, norm, size;

            // Are we ES1 or ES2?
            uint32_t esMajVer = GetESMajorVersion();
            Packetize(esMajVer);

            if (esMajVer == 1)
            {
               void *ptr;

               GLint v[4];
               Real(glGetIntegerv)(GL_ARRAY_BUFFER_BINDING, v);
               Packetize(v[0] != 0);

               if (v[0] == 0) // Need to send vertex array data along with this packet
               {
                  // Attach the data for any active ES1 client-side arrays
                  if (Real(glIsEnabled)(GL_VERTEX_ARRAY))
                  {
                     Real(glGetPointerv)(GL_VERTEX_ARRAY_POINTER, &ptr);
                     if (ptr != NULL)
                     {
                        Real(glGetIntegerv)(GL_VERTEX_ARRAY_SIZE, &size);
                        Real(glGetIntegerv)(GL_VERTEX_ARRAY_TYPE, &type);
                        Real(glGetIntegerv)(GL_VERTEX_ARRAY_STRIDE, &stride);

                        Packetize(GL_VERTEX_ARRAY_POINTER, size, type, stride);
                        if (stride != 0)
                           bytesPerVert = stride;
                        else
                           bytesPerVert = BytesForType(type) * size;
                        sCurPacket->AddItem(PacketItem(ptr, bytesPerVert * (first + count)));
                     }
                  }

                  if (Real(glIsEnabled)(GL_COLOR_ARRAY) &&
                      (!Real(glIsEnabled)(GL_LIGHTING) || Real(glIsEnabled)(GL_COLOR_MATERIAL)))
                  {
                     Real(glGetPointerv)(GL_COLOR_ARRAY_POINTER, &ptr);
                     if (ptr != NULL)
                     {
                        Real(glGetIntegerv)(GL_COLOR_ARRAY_SIZE, &size);
                        Real(glGetIntegerv)(GL_COLOR_ARRAY_TYPE, &type);
                        Real(glGetIntegerv)(GL_COLOR_ARRAY_STRIDE, &stride);

                        Packetize(GL_COLOR_ARRAY_POINTER, size, type, stride);
                        if (stride != 0)
                           bytesPerVert = stride;
                        else
                           bytesPerVert = BytesForType(type) * size;
                        sCurPacket->AddItem(PacketItem(ptr, bytesPerVert * (first + count)));
                     }
                  }

                  GLint numTUs = 1;
                  GLint curTU = 0;
                  Real(glGetIntegerv)(GL_MAX_TEXTURE_UNITS, &numTUs);
                  Real(glGetIntegerv)(GL_CLIENT_ACTIVE_TEXTURE, &curTU);

                  for (uint32_t tu = 0; tu < (uint32_t)numTUs; tu++)
                  {
                     Real(glClientActiveTexture(GL_TEXTURE0 + tu));

                     if (Real(glIsEnabled)(GL_TEXTURE_COORD_ARRAY))
                     {
                        Real(glGetPointerv)(GL_TEXTURE_COORD_ARRAY_POINTER, &ptr);
                        if (ptr != NULL)
                        {
                           Real(glGetIntegerv)(GL_TEXTURE_COORD_ARRAY_SIZE, &size);
                           Real(glGetIntegerv)(GL_TEXTURE_COORD_ARRAY_TYPE, &type);
                           Real(glGetIntegerv)(GL_TEXTURE_COORD_ARRAY_STRIDE, &stride);

                           Packetize(GL_TEXTURE_COORD_ARRAY_POINTER + tu, size, type, stride);
                           if (stride != 0)
                              bytesPerVert = stride;
                           else
                              bytesPerVert = BytesForType(type) * size;
                           sCurPacket->AddItem(PacketItem(ptr, bytesPerVert * (first + count)));
                        }
                     }
                  }
                  Real(glClientActiveTexture(curTU));

                  if (Real(glIsEnabled)(GL_NORMAL_ARRAY))
                  {
                     Real(glGetPointerv)(GL_NORMAL_ARRAY_POINTER, &ptr);
                     if (ptr != NULL)
                     {
                        size = 3;
                        Real(glGetIntegerv)(GL_NORMAL_ARRAY_TYPE, &type);
                        Real(glGetIntegerv)(GL_NORMAL_ARRAY_STRIDE, &stride);

                        Packetize(GL_NORMAL_ARRAY_POINTER, size, type, stride);
                        if (stride != 0)
                           bytesPerVert = stride;
                        else
                           bytesPerVert = BytesForType(type) * size;
                        sCurPacket->AddItem(PacketItem(ptr, bytesPerVert * (first + count)));
                     }
                  }

                  if (Real(glIsEnabled)(GL_POINT_SIZE_ARRAY_OES))
                  {
                     Real(glGetPointerv)(GL_POINT_SIZE_ARRAY_POINTER_OES, &ptr);
                     if (ptr != NULL)
                     {
                        size = 1;
                        Real(glGetIntegerv)(GL_POINT_SIZE_ARRAY_TYPE_OES, &type);
                        Real(glGetIntegerv)(GL_POINT_SIZE_ARRAY_STRIDE_OES, &stride);

                        Packetize(GL_POINT_SIZE_ARRAY_POINTER_OES, size, type, stride);
                        if (stride != 0)
                           bytesPerVert = stride;
                        else
                           bytesPerVert = BytesForType(type) * size;
                        sCurPacket->AddItem(PacketItem(ptr, bytesPerVert * (first + count)));
                     }
                  }

                  Packetize(-1);
               }
            }
            else if (esMajVer >= 2)
            {
               GLint currentProg;
               GLint maxAttribs;

               Real(glGetIntegerv)(GL_CURRENT_PROGRAM, &currentProg);
               Real(glGetProgramiv)(currentProg, GL_ACTIVE_ATTRIBUTES, &maxAttribs);

               for (GLint a = 0; a < maxAttribs; a++)
               {
                  void   *attribs = NULL;
                  GLint   binding;
                  GLint   enabled;
                  GLint   location;
                  GLint   size;
                  GLenum  atype;
                  GLchar  name[1024];
                  GLsizei len = 1024;

                  name[0] = '\0';

                  Real(glGetActiveAttrib)(currentProg, a, len, &len, &size, &atype, name);
                  location = Real(glGetAttribLocation)(currentProg, name);

                  if (location != -1)
                  {
                     Real(glGetVertexAttribiv)(location, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, &binding);
                     Real(glGetVertexAttribiv)(location, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &enabled);

                     if (binding == 0 && enabled)
                     {
                        Real(glGetVertexAttribPointerv)(location, GL_VERTEX_ATTRIB_ARRAY_POINTER, &attribs);
                        Real(glGetVertexAttribiv)(location, GL_VERTEX_ATTRIB_ARRAY_SIZE, &size);
                        Real(glGetVertexAttribiv)(location, GL_VERTEX_ATTRIB_ARRAY_TYPE, &type);
                        Real(glGetVertexAttribiv)(location, GL_VERTEX_ATTRIB_ARRAY_NORMALIZED, &norm);
                        Real(glGetVertexAttribiv)(location, GL_VERTEX_ATTRIB_ARRAY_STRIDE, &stride);

                        if (stride != 0)
                           bytesPerVert = stride;
                        else
                           bytesPerVert = BytesForType(type) * size;

                        if (attribs != NULL)
                        {
                           Packetize(a, size, type, norm, stride);
                           sCurPacket->AddItem(PacketItem(attribs, bytesPerVert * (first + count)));
                        }
                     }
                  }
               }

               Packetize(-1);
            }
         }
      }

      bool ret = PostPacketize(&p);
      if (ret)
      {
         TIMESTAMP start, end;
         plGetTime(&start);
         Real(glDrawArrays)(mode, first, count);
         plGetTime(&end);
         LogGLError();
         RetVoidFunc(plTimeDiffNano(&start, &end));
      }
   }
   else if (!sOrphaned && sBottleneckMode == Control::eNullDrawCalls &&
           (sEnabled || (sMinimalModeCmds.find(cmd_glDrawArrays) != sMinimalModeCmds.end())))
   {
      // Do nothing
   }
   else
      RealApiEvent(glDrawArrays, (mode, first, count));
}

static uint32_t FindMaxIndex(GLsizei count, GLenum type, const GLvoid* indices)
{
   // Work out what the max index is
   GLuint maxIndx = 0;
   GLint elementBind;
   Real(glGetIntegerv)(GL_ELEMENT_ARRAY_BUFFER_BINDING, &elementBind);

   GLboolean primRestart = Real(glIsEnabled(GL_PRIMITIVE_RESTART_FIXED_INDEX));

   if (elementBind == 0)
   {
      // Scan the client side index array
      int32_t i;
      if (type == GL_BYTE || type == GL_UNSIGNED_BYTE)
      {
         for (i = 0; i < count; i++)
         {
            if (!primRestart || (primRestart && ((uint8_t*)indices)[i] != 0xFF))
            {
               if (((uint8_t*)indices)[i] > maxIndx)
                  maxIndx = ((uint8_t*)indices)[i];
            }
         }
      }
      else if (type == GL_SHORT || type == GL_UNSIGNED_SHORT || type == GL_HALF_FLOAT)
      {
         for (i = 0; i < count; i++)
         {
            if (!primRestart || (primRestart && ((uint16_t*)indices)[i] != 0xFFFF))
            {
               if (((uint16_t*)indices)[i] > maxIndx)
                  maxIndx = ((uint16_t*)indices)[i];
            }
         }
      }
      else
      {
         for (i = 0; i < count; i++)
         {
            if (!primRestart || (primRestart && ((uint32_t*)indices)[i] != 0xFFFFFFFF))
            {
               if (((uint32_t*)indices)[i] > (GLuint)maxIndx)
                  maxIndx = (GLuint)((uint32_t*)indices)[i];
            }
         }
      }
   }
   else
   {
      // Need to use the VBO index buffer
      EGLContext curCtx = Real(eglGetCurrentContext)();

      Buffer *buffer = &sContexts[(uintptr_t)curCtx].m_buffers[elementBind];

      switch (type)
      {
      case GL_BYTE :
      case GL_UNSIGNED_BYTE :
         maxIndx = buffer->m_maxByteIndx;
         if (primRestart && maxIndx == 0xFF)
            maxIndx = buffer->m_nextHighestByteIndx;
         break;
      case GL_SHORT :
      case GL_UNSIGNED_SHORT :
      case GL_HALF_FLOAT :
         maxIndx = buffer->m_maxShortIndx;
         if (primRestart && maxIndx == 0xFFFF)
            maxIndx = buffer->m_nextHighestShortIndx;
         break;
      case GL_INT :
      case GL_UNSIGNED_INT :
      case GL_FIXED :
      case GL_INT_2_10_10_10_REV :
      case GL_UNSIGNED_INT_2_10_10_10_REV :
      case GL_FLOAT :
      default :
         maxIndx = buffer->m_maxIntIndx;
         if (primRestart && maxIndx == 0xFFFFFFFF)
            maxIndx = buffer->m_nextHighestIntIndx;
         break;
      }
   }

   return maxIndx;
}

DLLEXPORT void DLLEXPORTENTRY specialized_glDrawElements (GLenum mode, GLsizei count, GLenum type, const GLvoid* indices)
{
   APIInitAndLock locker(PL_FUNCTION);
   if (WantCommand(cmd_glDrawElements))
   {
      Packet   p(eAPI_FUNCTION);
      sCurPacket = &p;
      p.AddItem(cmd_glDrawElements);
      Packetize(mode, count, type, indices);

      if (sCaptureStream)
      {
         GLint elementBind;
         Real(glGetIntegerv)(GL_ELEMENT_ARRAY_BUFFER_BINDING, &elementBind);

         Packetize(elementBind != 0);

         // Do we need to send the client-side indices array?
         if (elementBind == 0)
         {
            uint32_t bytesPerIndex = 0;
            switch (type)
            {
            case GL_UNSIGNED_BYTE  : bytesPerIndex = 1; break;
            case GL_UNSIGNED_SHORT : bytesPerIndex = 2; break;
            case GL_UNSIGNED_INT   : bytesPerIndex = 4; break;
            }

            sCurPacket->AddItem(PacketItem((void*)indices, bytesPerIndex * count));
         }
         else
            sCurPacket->AddItem(PacketItem((void*)NULL, 0));


         // Are we ES1 or ES2?
         uint32_t esMajVer = GetESMajorVersion();
         Packetize(esMajVer);

         // Do we need to send vertex array data along with this packet
         {
            GLint typ, bytesPerVert, stride, norm, size;

            if (esMajVer == 1)
            {
               GLint arrayBind;
               void *ptr;

               Real(glGetIntegerv)(GL_ARRAY_BUFFER_BINDING, &arrayBind);
               Packetize(arrayBind != 0);

               if (arrayBind == 0)
               {
                  // Work out what the max index is (and therefore how many attribs to send)
                  GLint maxIndx = FindMaxIndex(count, type, indices);

                  // Ensure we get the last element
                  maxIndx += 1;

                  // Attach the data for any active ES1 client-side arrays
                  if (Real(glIsEnabled)(GL_VERTEX_ARRAY))
                  {
                     Real(glGetPointerv)(GL_VERTEX_ARRAY_POINTER, &ptr);
                     if (ptr != NULL)
                     {
                        Real(glGetIntegerv)(GL_VERTEX_ARRAY_SIZE, &size);
                        Real(glGetIntegerv)(GL_VERTEX_ARRAY_TYPE, &typ);
                        Real(glGetIntegerv)(GL_VERTEX_ARRAY_STRIDE, &stride);

                        Packetize(GL_VERTEX_ARRAY_POINTER, size, typ, stride);
                        if (stride != 0)
                           bytesPerVert = stride;
                        else
                           bytesPerVert = BytesForType(typ) * size;
                        sCurPacket->AddItem(PacketItem(ptr, bytesPerVert * maxIndx));
                     }
                  }

                  if (Real(glIsEnabled)(GL_COLOR_ARRAY) &&
                      (!Real(glIsEnabled)(GL_LIGHTING) || Real(glIsEnabled)(GL_COLOR_MATERIAL)))
                  {
                     Real(glGetPointerv)(GL_COLOR_ARRAY_POINTER, &ptr);
                     if (ptr != NULL)
                     {
                        Real(glGetIntegerv)(GL_COLOR_ARRAY_SIZE, &size);
                        Real(glGetIntegerv)(GL_COLOR_ARRAY_TYPE, &typ);
                        Real(glGetIntegerv)(GL_COLOR_ARRAY_STRIDE, &stride);

                        Packetize(GL_COLOR_ARRAY_POINTER, size, typ, stride);
                        if (stride != 0)
                           bytesPerVert = stride;
                        else
                           bytesPerVert = BytesForType(typ) * size;
                        sCurPacket->AddItem(PacketItem(ptr, bytesPerVert * maxIndx));
                     }
                  }

                  GLint numTUs = 1;
                  GLint curTU = 0;
                  Real(glGetIntegerv)(GL_MAX_TEXTURE_UNITS, &numTUs);
                  Real(glGetIntegerv)(GL_CLIENT_ACTIVE_TEXTURE, &curTU);

                  for (uint32_t tu = 0; tu < (uint32_t)numTUs; tu++)
                  {
                     Real(glClientActiveTexture(GL_TEXTURE0 + tu));

                     if (Real(glIsEnabled)(GL_TEXTURE_COORD_ARRAY))
                     {
                        Real(glGetPointerv)(GL_TEXTURE_COORD_ARRAY_POINTER, &ptr);
                        if (ptr != NULL)
                        {
                           Real(glGetIntegerv)(GL_TEXTURE_COORD_ARRAY_SIZE, &size);
                           Real(glGetIntegerv)(GL_TEXTURE_COORD_ARRAY_TYPE, &typ);
                           Real(glGetIntegerv)(GL_TEXTURE_COORD_ARRAY_STRIDE, &stride);

                           Packetize(GL_TEXTURE_COORD_ARRAY_POINTER + tu, size, typ, stride);
                           if (stride != 0)
                              bytesPerVert = stride;
                           else
                              bytesPerVert = BytesForType(typ) * size;
                           sCurPacket->AddItem(PacketItem(ptr, bytesPerVert * maxIndx));
                        }
                     }
                  }
                  Real(glClientActiveTexture(curTU));

                  if (Real(glIsEnabled)(GL_NORMAL_ARRAY))
                  {
                     Real(glGetPointerv)(GL_NORMAL_ARRAY_POINTER, &ptr);
                     if (ptr != NULL)
                     {
                        size = 3;
                        Real(glGetIntegerv)(GL_NORMAL_ARRAY_TYPE, &typ);
                        Real(glGetIntegerv)(GL_NORMAL_ARRAY_STRIDE, &stride);

                        Packetize(GL_NORMAL_ARRAY_POINTER, size, typ, stride);
                        if (stride != 0)
                           bytesPerVert = stride;
                        else
                           bytesPerVert = BytesForType(typ) * size;
                        sCurPacket->AddItem(PacketItem(ptr, bytesPerVert * maxIndx));
                     }
                  }

                  if (Real(glIsEnabled)(GL_POINT_SIZE_ARRAY_OES))
                  {
                     Real(glGetPointerv)(GL_POINT_SIZE_ARRAY_POINTER_OES, &ptr);
                     if (ptr != NULL)
                     {
                        size = 1;
                        Real(glGetIntegerv)(GL_POINT_SIZE_ARRAY_TYPE_OES, &typ);
                        Real(glGetIntegerv)(GL_POINT_SIZE_ARRAY_STRIDE_OES, &stride);

                        Packetize(GL_POINT_SIZE_ARRAY_POINTER_OES, size, typ, stride);
                        if (stride != 0)
                           bytesPerVert = stride;
                        else
                           bytesPerVert = BytesForType(typ) * size;
                        sCurPacket->AddItem(PacketItem(ptr, bytesPerVert * maxIndx));
                     }
                  }

                  Packetize(-1);
               }
            }
            else if (esMajVer >= 2)
            {
               GLint maxIndx = -1;

               GLint currentProg;
               GLint maxAttribs;

               Real(glGetIntegerv)(GL_CURRENT_PROGRAM, &currentProg);
               Real(glGetProgramiv)(currentProg, GL_ACTIVE_ATTRIBUTES, &maxAttribs);

               for (GLint a = 0; a < maxAttribs; a++)
               {
                  void   *attribs = NULL;
                  GLint   binding;
                  GLint   enabled;
                  GLint   location;
                  GLint   size;
                  GLenum  atype;
                  GLchar  name[1024];
                  GLsizei len = 1024;

                  name[0] = '\0';

                  Real(glGetIntegerv)(GL_CURRENT_PROGRAM, &currentProg);
                  Real(glGetActiveAttrib)(currentProg, a, len, &len, &size, &atype, name);
                  location = Real(glGetAttribLocation)(currentProg, name);

                  if (location != -1)
                  {
                     Real(glGetVertexAttribiv)(location, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, &binding);
                     Real(glGetVertexAttribiv)(location, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &enabled);

                     if (binding == 0 && enabled)
                     {
                        if (maxIndx == -1)
                        {
                           // Work out what the max index is (and therefore how many attribs to send)
                           maxIndx = FindMaxIndex(count, type, indices);

                           // Ensure we get the last element
                           maxIndx += 1;
                        }

                        Real(glGetVertexAttribPointerv)(location, GL_VERTEX_ATTRIB_ARRAY_POINTER, &attribs);
                        Real(glGetVertexAttribiv)(location, GL_VERTEX_ATTRIB_ARRAY_SIZE, &size);
                        Real(glGetVertexAttribiv)(location, GL_VERTEX_ATTRIB_ARRAY_TYPE, &typ);
                        Real(glGetVertexAttribiv)(location, GL_VERTEX_ATTRIB_ARRAY_NORMALIZED, &norm);
                        Real(glGetVertexAttribiv)(location, GL_VERTEX_ATTRIB_ARRAY_STRIDE, &stride);

                        if (stride != 0)
                           bytesPerVert = stride;
                        else
                           bytesPerVert = BytesForType(typ) * size;

                        if (attribs != NULL)
                        {
                           Packetize(location, size, typ, norm, stride);
                           sCurPacket->AddItem(PacketItem(attribs, bytesPerVert * maxIndx));
                        }
                     }
                  }
               }

               Packetize(-1);
            }
         }
      }

      bool ret = PostPacketize(&p);
      if (ret)
      {
         TIMESTAMP start, end;
         plGetTime(&start);
         Real(glDrawElements)(mode, count, type, indices);
         plGetTime(&end);
         LogGLError();
         RetVoidFunc(plTimeDiffNano(&start, &end));
      }
   }
   else if (!sOrphaned && sBottleneckMode == Control::eNullDrawCalls &&
           (sEnabled || (sMinimalModeCmds.find(cmd_glDrawElements) != sMinimalModeCmds.end())))
   {
      // Do nothing
   }
   else
      RealApiEvent(glDrawElements, (mode, count, type, indices));
}

DLLEXPORT void DLLEXPORTENTRY specialized_glFinish (void)
{
   APIInitAndLock locker(PL_FUNCTION);
   if (!sOrphaned)
   {
      sFrameCnt++;
      sTotalFrameCnt++;

      // Poll the event timeline from the driver & kernel. Note: this does not send the event
      // data to GPUMonitor.
      CollectLowerLevelEventData();

      Packet   p(eAPI_FUNCTION);
      sCurPacket = &p;
      p.AddItem(cmd_glFinish);
      bool ret = PostPacketize(&p);
      sSendNextRet = true;
      if (ret)
      {
         TIMESTAMP start, end;
         plGetTime(&start);
         RealApiEvent(glFinish, ());
         plGetTime(&end);
         LogGLError();
         RetVoidFunc(plTimeDiffNano(&start, &end));
      }
   }
   else
      RealApiEvent(glFinish, ());
}

DLLEXPORT void DLLEXPORTENTRY specialized_glFogfv (GLenum pname, const GLfloat *params)
   FuncExtra1(GL, glFogfv, (pname, params), ((void*)params, (pname == GL_FOG_COLOR ? 4 : 1) * sizeof(GLfloat)))
DLLEXPORT void DLLEXPORTENTRY specialized_glFogxv (GLenum pname, const GLfixed *params)
   FuncExtra1(GL, glFogxv, (pname, params), ((void*)params, (pname == GL_FOG_COLOR ? 4 : 1) * sizeof(GLfixed)))
DLLEXPORT void DLLEXPORTENTRY specialized_glLightModelfv (GLenum pname, const GLfloat *params)
   FuncExtra1(GL, glLightModelfv, (pname, params), ((void*)params, (pname == GL_LIGHT_MODEL_AMBIENT ? 4 : 1) * sizeof(GLfloat)))
DLLEXPORT void DLLEXPORTENTRY specialized_glLightModelxv (GLenum pname, const GLfixed *params)
   FuncExtra1(GL, glLightModelxv, (pname, params), ((void*)params, (pname == GL_LIGHT_MODEL_AMBIENT ? 4 : 1) * sizeof(GLfixed)))
DLLEXPORT void DLLEXPORTENTRY specialized_glLightfv (GLenum light, GLenum pname, const GLfloat *params)
   FuncExtra1(GL, glLightfv, (light, pname, params), ((void*)params, LightParamsSize(pname) * sizeof(GLfloat)))
DLLEXPORT void DLLEXPORTENTRY specialized_glLightxv (GLenum light, GLenum pname, const GLfixed *params)
   FuncExtra1(GL, glLightxv, (light, pname, params), ((void*)params, LightParamsSize(pname) * sizeof(GLfixed)))
DLLEXPORT void DLLEXPORTENTRY specialized_glLoadMatrixf (const GLfloat *m)
   FuncExtra1(GL, glLoadMatrixf, (m), ((void*)m, 16 * sizeof(GLfloat)))
DLLEXPORT void DLLEXPORTENTRY specialized_glLoadMatrixx (const GLfixed *m)
   FuncExtra1(GL, glLoadMatrixx, (m), ((void*)m, 16 * sizeof(GLfixed)))
DLLEXPORT void DLLEXPORTENTRY specialized_glMaterialfv (GLenum face, GLenum pname, const GLfloat *params)
   FuncExtra1(GL, glMaterialfv, (face, pname, params), ((void*)params, MaterialParamsSize(pname) * sizeof(GLfloat)))
DLLEXPORT void DLLEXPORTENTRY specialized_glMaterialxv (GLenum face, GLenum pname, const GLfixed *params)
   FuncExtra1(GL, glMaterialxv, (face, pname, params), ((void*)params, MaterialParamsSize(pname) * sizeof(GLfixed)))
DLLEXPORT void DLLEXPORTENTRY specialized_glMultMatrixf (const GLfloat *m)
   FuncExtra1(GL, glMultMatrixf, (m), ((void*)m, 16 * sizeof(GLfloat)))
DLLEXPORT void DLLEXPORTENTRY specialized_glMultMatrixx (const GLfixed *m)
   FuncExtra1(GL, glMultMatrixx, (m), ((void*)m, 16 * sizeof(GLfixed)))
DLLEXPORT void DLLEXPORTENTRY specialized_glNormalPointer (GLenum type, GLsizei stride, const GLvoid *pointer)
{
   APIInitAndLock locker(PL_FUNCTION);
   if (WantCommand(cmd_glNormalPointer))
   {
      Packet   p(eAPI_FUNCTION);
      sCurPacket = &p;
      p.AddItem(cmd_glNormalPointer);
      Packetize(type, stride, pointer);

      if (sCaptureStream)
      {
         GLint v[4];
         Real(glGetIntegerv)(GL_ARRAY_BUFFER_BINDING, v);
         Packetize(v[0] != 0);
      }

      bool ret = PostPacketize(&p);
      if (ret)
      {
         TIMESTAMP start, end;
         plGetTime(&start);
         Real(glNormalPointer)(type, stride, pointer);
         plGetTime(&end);
         LogGLError();
         RetVoidFunc(plTimeDiffNano(&start, &end));
      }
   }
   else
      RealApiEvent(glNormalPointer, (type, stride, pointer));
}
DLLEXPORT void DLLEXPORTENTRY specialized_glPointParameterfv (GLenum pname, const GLfloat *params)
   FuncExtra1(GL, glPointParameterfv, (pname, params), ((void*)params, (pname == GL_POINT_DISTANCE_ATTENUATION ? 3 : 1) * sizeof(GLfloat)))
DLLEXPORT void DLLEXPORTENTRY specialized_glPointParameterxv (GLenum pname, const GLfixed *params)
   FuncExtra1(GL, glPointParameterxv, (pname, params), ((void*)params, (pname == GL_POINT_DISTANCE_ATTENUATION ? 3 : 1) * sizeof(GLfixed)))
DLLEXPORT void DLLEXPORTENTRY specialized_glShaderBinary (GLsizei n, const GLuint* shaders, GLenum binaryformat, const GLvoid* binary, GLsizei length)
{
   APIInitAndLock locker(PL_FUNCTION);
   if (WantCommand(cmd_glShaderBinary))
   {
      Packet   p(eAPI_FUNCTION);
      sCurPacket = &p;
      p.AddItem(cmd_glShaderBinary);
      Packetize(n, shaders, binaryformat, binary, length);

      if (sCaptureStream)
      {
         for (int i = 0; i < n; i++)
            Packetize(shaders[i]);
         Packetize(binary, length);
      }

      bool ret = PostPacketize(&p);
      if (ret)
      {
         TIMESTAMP start, end;
         plGetTime(&start);
         Real(glShaderBinary)(n, shaders, binaryformat, binary, length);
         plGetTime(&end);
         LogGLError();
         RetVoidFunc(plTimeDiffNano(&start, &end));
      }
   }
   else
      RealApiEvent(glShaderBinary, (n, shaders, binaryformat, binary, length));
}

static bool GetOutTypeAndName(const GLchar *s, std::string *type, std::string *name)
{
   // Read tokens and a ; - [prec] type name
   uint32_t    state = 0;
   std::string token[3];
   uint32_t    t = 0;

   while (*s != '\0')
   {
      GLchar ch = *s;
      if (state == 0)   // Skip whitespace
      {
         if (!isspace(ch))
            state = 1;
         else
            s++;
      }
      else if (state == 1) // Get token
      {
         if (ch == ';')
            break;
         else if (ch == ')' || ch == ',' || ch == '(' || ch == '{' || ch == '}')
            return false;
         else if (!isspace(ch))
         {
            if (t >= 3)
               return false;

            token[t] += ch;
            s++;
         }
         else
         {
            t++;
            state = 0;
         }
      }
   }

   if (token[0] == "highp" || token[0] == "lowp" || token[0] == "mediump")
   {
      token[0] = token[1];
      token[1] = token[2];
   }

   if (token[1] == "highp" || token[1] == "lowp" || token[1] == "mediump")
      token[1] = token[2];

   if (token[0] == "" || token[1] == "")
      return false;

   *type = token[0];
   *name = token[1];

   return true;
}

static bool IsSeparator(char c)
{
   return c == ';' || isspace(c);
}

static std::string PreProcessShader(const std::string &str)
{
   uint32_t    state = 0;
   std::string ret;

   for (uint32_t c = 0; c < str.length(); c++)
   {
      const GLchar *s = str.c_str() + c;
      GLchar ch = *s;

      switch (state)
      {
         case 0 :
         {
            if (!strncmp(s, "/*", 2))
               state = 1;  // In block comment
            else if (!strncmp(s, "//", 2))
               state = 2;  // In line comment
            else
               ret += ch;
            break;
         }
         case 1 : // In block comment - wait for */
         {
            if (!strncmp(s, "*/", 2))
            {
               c++;
               state = 0;
            }
            break;
         }
         case 2: // In line comment - wait for \n
         {
            if (ch == '\n')
            {
               state = 0;
               ret += ch;
            }
            break;
         }
      }
   }

   return ret;
}

static std::string GetOutputOverride(const std::string &name, const std::string &type, uint32_t mode, float f)
{
   std::string                 params;
   std::array<std::string, 4>  fvals;
   std::array<std::string, 4>  ivals;
   std::array<std::string, 4>  bvals;

   bvals = { "false", "false", "false", "true" };

   if (mode == Control::eOverdraw)
   {
      fvals = { "0.0", "0.1", "0.0", "1.0" };
      ivals = { "0",   "0",   "0",   "255" };
   }
   else if (mode == Control::eMinimalFragShader)
   {
      char str[64];
      snprintf(str, 64, "%f", f);
      fvals = { str, "0.5", "0.5", "1.0" };
      snprintf(str, 64, "%d", (int)(f * 255.0f));
      ivals = { str, "0", "0", "255" };
   }
   else
      assert(0);

   char last  = type.length() > 0 ? type[type.length() - 1] : '\0';
   char first = type.length() > 0 ? type[0] : '\0';

   std::array<std::string, 4> *vals;

   if (first == 'i' || first == 'u')
      vals = &ivals;
   else if (first == 'b')
      vals = &bvals;
   else
      vals = &fvals;

   switch (last)
   {
   case '4':  params = (*vals)[0] + "," + (*vals)[1] + "," + (*vals)[2] + "," + (*vals)[3]; break;
   case '3':  params = (*vals)[0] + "," + (*vals)[1] + "," + (*vals)[2]; break;
   case '2':  params = (*vals)[0] + "," + (*vals)[1]; break;
   default:   params = (*vals)[0]; break;
   }

   return name + " = " + type + "(" + params + ");\n";
}

static void ReplaceFragmentShader(GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length)
{
   int32_t  i;
   uint32_t c;
   uint32_t state = 0;  /* Awaiting # */
   GLchar   *buf = 0;
   GLchar   *ptr = 0;
   uint32_t braces = 0;

   std::vector<std::string>   outTypes;
   std::vector<std::string>   outNames;

   std::string shaderStr;

   for (i = 0; i < count; i++)
   {
      if (string[i])
      {
         if (length && length[i])
            shaderStr += std::string(string[i], length[i]);
         else
            shaderStr += std::string(string[i]);
      }
   }

   shaderStr = PreProcessShader(shaderStr);

   buf = (GLchar*)malloc(shaderStr.length() + 1024);
   ptr = buf;

   memset(buf, 0, shaderStr.length() + 1024);

   /* Minimize the shader */
   GLchar *d = ptr;
   for (c = 0; c < shaderStr.length(); c++)
   {
      const GLchar *s = shaderStr.c_str() + c;
      GLchar ch = *s;

      switch (state)
      {
      case 0: // Awaiting # (if present)
      {
         if (!isspace(ch) && ch != '#')
            state = 2;
         else if (ch == '#')
            state = 1;
         *d++ = ch;
         break;
      }
      case 1: // Awaiting version
      {
         if (!strncmp(s, "version", 7))
         {
            c += 6;
            state = 2;
            memcpy(d, s, 7);
            d += 7;
         }
         else if (!isspace(ch))
         {
            state = 2;
            *d++ = ch;
         }
         else
            *d++ = ch;
         break;
      }
      case 2: /* Awaiting main or layout or out */
      {
         if (!strncmp(s, "layout", 6) && IsSeparator(*(s-1)))
         {
            c += 5;
            memcpy(d, s, 6);
            d += 6;
         }
         else if (!strncmp(s, "out", 3) && IsSeparator(*(s-1)))
         {
            c += 2;
            memcpy(d, s, 3);
            d += 3;
            s += 3;

            if (isspace(*s))
            {
               // We have an 'out' token
               std::string type, name;
               if (GetOutTypeAndName(s, &type, &name))
               {
                  outTypes.push_back(type);
                  outNames.push_back(name);
               }
            }
         }
         else if (!strncmp(s, "main", 4) && IsSeparator(*(s-1)))
         {
            state = 4;
            c += 3;
            memcpy(d, s, 4);
            d += 4;
         }
         else
            *d++ = ch;
         break;
      }
      case 4:  // Check (
      {
         if (!isspace(ch) && ch != '(')
            state = 2;
         else if (!isspace(ch) && ch == '(')
            state = 5;
         *d++ = ch;
         break;
      }
      case 5: // ...)
      {
         if (ch == ')')
            state = 6;
         *d++ = ch;
         break;
      }
      case 6: // We have now seen main()
      {
         // Now we need to look for the final '}'
         if (ch == '{')
         {
            braces++;
            *d++ = ch;
         }
         else if (ch == '}')
         {
            braces--;
            if (braces == 0)
               state = 7;
            else
               *d++ = ch;
         }
         else
            *d++ = ch;
         break;
      }
      case 7:  // Add final output overrides and finish
      {
         assert(outTypes.size() == outNames.size());

         if (outTypes.size() == 0)
         {
            if (shaderStr.find("gl_FragColor") != std::string::npos)
            {
               outTypes.push_back("vec4");
               outNames.push_back("gl_FragColor");
            }
            else if (shaderStr.find("gl_FragData") != std::string::npos)
            {
               outTypes.push_back("vec4");
               outNames.push_back("gl_FragData[0]");
            }
         }

         *d = '\0';

         if (sBottleneckMode == Control::eOverdraw)
         {
            Real(glEnable)(GL_BLEND);
            Real(glBlendFunc)(GL_ONE, GL_ONE);
            Real(glBlendEquation)(GL_FUNC_ADD);
            for (uint32_t i = 0; i < outTypes.size(); i++)
            {
               strcat(d, GetOutputOverride(outNames[i].c_str(), outTypes[i].c_str(), sBottleneckMode, 0.0f).c_str());
            }
            strcat(d, "}\n");
         }
         else if (sBottleneckMode == Control::eMinimalFragShader)
         {
            for (uint32_t i = 0; i < outTypes.size(); i++)
            {
               strcat(d, GetOutputOverride(outNames[i].c_str(), outTypes[i].c_str(), sBottleneckMode,
                      (float)(shader % 255) / 255.0f).c_str());
            }
            strcat(d, "}\n");
         }

         d = buf + strlen(buf);

         state = 8;
         break;
      }
      case 8:  // All done - just copy everything after main
      {
         *d++ = ch;
         break;
      }
      default:
         assert(0);
      }
   }

   Real(glShaderSource)(shader, 1, (const GLchar **)&buf, NULL);
   Real(glCompileShader)(shader);

   GLint status;
   Real(glGetShaderiv)(shader, GL_COMPILE_STATUS, &status);

   if (status == GL_FALSE) // Put the original shader back if we failed to compile
   {
      GLchar log[2048];
      Real(glGetShaderInfoLog)(shader, 2048, NULL, log);
      Real(glShaderSource)(shader, count, string, length);

      //printf("\n\n================= FAILED\nORIG\n%s\n==============\nNEW\n%s\n===========%s\n\n",
      //       (char*)*string, buf, log);
   }

   free(buf);
}

DLLEXPORT void DLLEXPORTENTRY specialized_glShaderSource (GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length)
{
   APIInitAndLock locker(PL_FUNCTION);
   if (WantCommand(cmd_glShaderSource))
   {
      Packet   p(eAPI_FUNCTION);
      int32_t  i;

      sCurPacket = &p;
      p.AddItem(cmd_glShaderSource);
      Packetize(shader, count, string, length);

      if (length == NULL)
      {
         // Strings are NULL terminated
         for (i = 0; i < count; i++)
            sCurPacket->AddItem(PacketItem((void*)string[i], strlen(string[i])));
      }
      else
      {
         for (i = 0; i < count; i++)
         {
            int32_t len = length[i];
            if (len > 0)
               sCurPacket->AddItem(PacketItem((void*)string[i], len));
            else
            {
               // Some apps say they are going to download more strings than they do (GLBenchmark 2.7)
               if (string[i])
                  sCurPacket->AddItem(PacketItem((void*)string[i], strlen(string[i])));
               else
                  sCurPacket->AddItem(PacketItem((void*)"", strlen("")));
            }
         }
      }

      bool ret = PostPacketize(&p);
      if (ret)
      {
         TIMESTAMP start, end;
         plGetTime(&start);
         Real(glShaderSource)(shader, count, (const GLchar **)string, length);
         plGetTime(&end);
         LogGLError();
         RetVoidFunc(plTimeDiffNano(&start, &end));
      }
   }
   else if (!sOrphaned && (sEnabled || (sMinimalModeCmds.find(cmd_glShaderSource) != sMinimalModeCmds.end())) &&
            (sBottleneckMode == Control::eOverdraw || sBottleneckMode == Control::eMinimalFragShader))
   {
      GLint   type;
      Real(glGetShaderiv)(shader, GL_SHADER_TYPE, &type);

      if (type == GL_FRAGMENT_SHADER)
         ReplaceFragmentShader(shader, count, string, length);
      else
         RealApiEvent(glShaderSource, (shader, count, (const GLchar **)string, length));
   }
   else
   {
      RealApiEvent(glShaderSource, (shader, count, (const GLchar **)string, length));
   }
}
DLLEXPORT void DLLEXPORTENTRY specialized_glTexCoordPointer (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
   APIInitAndLock locker(PL_FUNCTION);
   if (WantCommand(cmd_glTexCoordPointer))
   {
      Packet   p(eAPI_FUNCTION);
      sCurPacket = &p;
      p.AddItem(cmd_glTexCoordPointer);
      Packetize(size, type, stride, pointer);

      if (sCaptureStream)
      {
         GLint v[4];
         Real(glGetIntegerv)(GL_ARRAY_BUFFER_BINDING, v);
         Packetize(v[0] != 0);
      }

      bool ret = PostPacketize(&p);
      if (ret)
      {
         TIMESTAMP start, end;
         plGetTime(&start);
         Real(glTexCoordPointer)(size, type, stride, pointer);
         plGetTime(&end);
         LogGLError();
         RetVoidFunc(plTimeDiffNano(&start, &end));
      }
   }
   else
      RealApiEvent(glTexCoordPointer, (size, type, stride, pointer));
}
DLLEXPORT void DLLEXPORTENTRY specialized_glTexEnvfv (GLenum target, GLenum pname, const GLfloat *params)
   FuncExtra1(GL, glTexEnvfv, (target, pname, params), ((void*)params, (pname == GL_TEXTURE_ENV_COLOR ? 4 : 1) * sizeof(GLfloat)))
DLLEXPORT void DLLEXPORTENTRY specialized_glTexEnviv (GLenum target, GLenum pname, const GLint *params)
   FuncExtra1(GL, glTexEnviv, (target, pname, params), ((void*)params, (pname == GL_TEXTURE_ENV_COLOR ? 4 : 1) * sizeof(GLint)))
DLLEXPORT void DLLEXPORTENTRY specialized_glTexEnvxv (GLenum target, GLenum pname, const GLfixed *params)
   FuncExtra1(GL, glTexEnvxv, (target, pname, params), ((void*)params, (pname == GL_TEXTURE_ENV_COLOR ? 4 : 1) * sizeof(GLfixed)))
DLLEXPORT void DLLEXPORTENTRY specialized_glTexImage2D (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels)
   TextureFuncExtra1MaybePBO(GL, glTexImage2D, (target, level, internalformat, width, height, border, format, type, pixels),
                      pixels, TextureSize(width, height, format, type, UnpackAlignment()), target, level, TinyTexImage2D)
DLLEXPORT void DLLEXPORTENTRY specialized_glTexParameterfv (GLenum target, GLenum pname, const GLfloat *params)
   FuncExtra1(GL, glTexParameterfv, (target, pname, params), ((void*)params, sizeof(GLfloat)))
DLLEXPORT void DLLEXPORTENTRY specialized_glTexParameteriv (GLenum target, GLenum pname, const GLint *params)
   FuncExtra1(GL, glTexParameteriv, (target, pname, params), ((void*)params, sizeof(GLint)))
DLLEXPORT void DLLEXPORTENTRY specialized_glTexParameterxv (GLenum target, GLenum pname, const GLfixed *params)
   FuncExtra1(GL, glTexParameterxv, (target, pname, params), ((void*)params, sizeof(GLfixed)))
DLLEXPORT void DLLEXPORTENTRY specialized_glTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* pixels)
   TextureFuncExtra1MaybePBO(GL, glTexSubImage2D, (target, level, xoffset, yoffset, width, height, format, type, pixels),
                      pixels, TextureSize(width, height, format, type, UnpackAlignment()), target, level, TinyTexSubImage2D)
DLLEXPORT void DLLEXPORTENTRY specialized_glUniform1fv (GLint location, GLsizei count, const GLfloat* v)
   FuncExtra1(GL, glUniform1fv, (location, count, v), ((void*)v, count * sizeof(GLfloat)))
DLLEXPORT void DLLEXPORTENTRY specialized_glUniform1i (GLint location, GLint x) Func(GL, glUniform1i, (location, x))
DLLEXPORT void DLLEXPORTENTRY specialized_glUniform1iv (GLint location, GLsizei count, const GLint* v)
   FuncExtra1(GL, glUniform1iv, (location, count, v), ((void*)v, count * sizeof(GLint)))
DLLEXPORT void DLLEXPORTENTRY specialized_glUniform2fv (GLint location, GLsizei count, const GLfloat* v)
   FuncExtra1(GL, glUniform2fv, (location, count, v), ((void*)v, 2 * count * sizeof(GLfloat)))
DLLEXPORT void DLLEXPORTENTRY specialized_glUniform2iv (GLint location, GLsizei count, const GLint* v)
   FuncExtra1(GL, glUniform2iv, (location, count, v), ((void*)v, 2 * count * sizeof(GLint)))
DLLEXPORT void DLLEXPORTENTRY specialized_glUniform3fv (GLint location, GLsizei count, const GLfloat* v)
   FuncExtra1(GL, glUniform3fv, (location, count, v), ((void*)v, 3 * count * sizeof(GLfloat)))
DLLEXPORT void DLLEXPORTENTRY specialized_glUniform3iv (GLint location, GLsizei count, const GLint* v)
   FuncExtra1(GL, glUniform3iv, (location, count, v), ((void*)v, 3 * count * sizeof(GLint)))
DLLEXPORT void DLLEXPORTENTRY specialized_glUniform4fv (GLint location, GLsizei count, const GLfloat* v)
   FuncExtra1(GL, glUniform4fv, (location, count, v), ((void*)v, 4 * count * sizeof(GLfloat)))
DLLEXPORT void DLLEXPORTENTRY specialized_glUniform4iv (GLint location, GLsizei count, const GLint* v)
   FuncExtra1(GL, glUniform4iv, (location, count, v), ((void*)v, 4 * count * sizeof(GLint)))
DLLEXPORT void DLLEXPORTENTRY specialized_glUniformMatrix2fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
   FuncExtra1(GL, glUniformMatrix2fv, (location, count, transpose, value), ((void*)value, 4 * count * sizeof(GLfloat)))
DLLEXPORT void DLLEXPORTENTRY specialized_glUniformMatrix3fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
   FuncExtra1(GL, glUniformMatrix3fv, (location, count, transpose, value), ((void*)value, 9 * count * sizeof(GLfloat)))
DLLEXPORT void DLLEXPORTENTRY specialized_glUniformMatrix4fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
   FuncExtra1(GL, glUniformMatrix4fv, (location, count, transpose, value), ((void*)value, 16 * count * sizeof(GLfloat)))
DLLEXPORT void DLLEXPORTENTRY specialized_glVertexAttrib1fv (GLuint indx, const GLfloat* values)
   FuncExtra1(GL, glVertexAttrib1fv, (indx, values), ((void*)values, 1 * sizeof(GLfloat)))
DLLEXPORT void DLLEXPORTENTRY specialized_glVertexAttrib2fv (GLuint indx, const GLfloat* values)
   FuncExtra1(GL, glVertexAttrib2fv, (indx, values), ((void*)values, 2 * sizeof(GLfloat)))
DLLEXPORT void DLLEXPORTENTRY specialized_glVertexAttrib3fv (GLuint indx, const GLfloat* values)
   FuncExtra1(GL, glVertexAttrib3fv, (indx, values), ((void*)values, 3 * sizeof(GLfloat)))
DLLEXPORT void DLLEXPORTENTRY specialized_glVertexAttrib4fv (GLuint indx, const GLfloat* values)
   FuncExtra1(GL, glVertexAttrib4fv, (indx, values), ((void*)values, 4 * sizeof(GLfloat)))
DLLEXPORT void DLLEXPORTENTRY specialized_glVertexAttribPointer (GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* ptr)
{
   APIInitAndLock locker(PL_FUNCTION);
   if (WantCommand(cmd_glVertexAttribPointer))
   {
      Packet   p(eAPI_FUNCTION);
      sCurPacket = &p;
      p.AddItem(cmd_glVertexAttribPointer);
      Packetize(indx, size, type, normalized, stride, ptr);

      if (sCaptureStream)
      {
         GLint v[4];
         Real(glGetIntegerv)(GL_ARRAY_BUFFER_BINDING, v);
         Packetize(v[0] != 0);
      }

      bool ret = PostPacketize(&p);
      if (ret)
      {
         TIMESTAMP start, end;
         plGetTime(&start);
         Real(glVertexAttribPointer)(indx, size, type, normalized, stride, ptr);
         plGetTime(&end);
         LogGLError();
         RetVoidFunc(plTimeDiffNano(&start, &end));
      }
   }
   else
      RealApiEvent(glVertexAttribPointer, (indx, size, type, normalized, stride, ptr));
}
DLLEXPORT void DLLEXPORTENTRY specialized_glVertexPointer (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
   APIInitAndLock locker(PL_FUNCTION);
   if (WantCommand(cmd_glVertexPointer))
   {
      Packet   p(eAPI_FUNCTION);
      sCurPacket = &p;
      p.AddItem(cmd_glVertexPointer);
      Packetize(size, type, stride, pointer);

      if (sCaptureStream)
      {
         GLint v[4];
         Real(glGetIntegerv)(GL_ARRAY_BUFFER_BINDING, v);
         Packetize(v[0] != 0);
      }

      bool ret = PostPacketize(&p);
      if (ret)
      {
         TIMESTAMP start, end;
         plGetTime(&start);
         Real(glVertexPointer)(size, type, stride, pointer);
         plGetTime(&end);
         LogGLError();
         RetVoidFunc(plTimeDiffNano(&start, &end));
      }
   }
   else
      RealApiEvent(glVertexPointer, (size, type, stride, pointer));
}
DLLEXPORT void DLLEXPORTENTRY specialized_glViewport (GLint x, GLint y, GLsizei width, GLsizei height)
{
   APIInitAndLock locker(PL_FUNCTION);
   if (WantCommand(cmd_glViewport))
   {
      Packet   p(eAPI_FUNCTION);
      sCurPacket = &p;
      p.AddItem(cmd_glViewport);
      Packetize(x, y, width, height);
      bool ret = PostPacketize(&p);
      if (ret)
      {
         TIMESTAMP start, end;
         plGetTime(&start);
         Real(glViewport)(x, y, width, height);
         plGetTime(&end);
         LogGLError();
         RetVoidFunc(plTimeDiffNano(&start, &end));
      }
   }
   else if (!sOrphaned && (sEnabled || (sMinimalModeCmds.find(cmd_glViewport) != sMinimalModeCmds.end())) &&
            sBottleneckMode == Control::eTinyViewport)
   {
      Real(glViewport)(0, 0, 1, 1);
   }
   else
      RealApiEvent(glViewport, (x, y, width, height));
}

/* GL Extensions */
DLLEXPORT void DLLEXPORTENTRY specialized_glDrawTexsOES (GLshort x, GLshort y, GLshort z, GLshort width, GLshort height) DrawFunc(GL, glDrawTexsOES, (x, y, z, width, height))
DLLEXPORT void DLLEXPORTENTRY specialized_glDrawTexiOES (GLint x, GLint y, GLint z, GLint width, GLint height) DrawFunc(GL, glDrawTexiOES, (x, y, z, width, height))
DLLEXPORT void DLLEXPORTENTRY specialized_glDrawTexxOES (GLfixed x, GLfixed y, GLfixed z, GLfixed width, GLfixed height) DrawFunc(GL, glDrawTexxOES, (x, y, z, width, height))
DLLEXPORT void DLLEXPORTENTRY specialized_glDrawTexsvOES (const GLshort *coords)
   DrawFuncExtra1(GL, glDrawTexsvOES, (coords), ((void*)coords, 5 * sizeof(GLshort)))
DLLEXPORT void DLLEXPORTENTRY specialized_glDrawTexivOES (const GLint *coords)
   DrawFuncExtra1(GL, glDrawTexivOES, (coords), ((void*)coords, 5 * sizeof(GLint)))
DLLEXPORT void DLLEXPORTENTRY specialized_glDrawTexxvOES (const GLfixed *coords)
   DrawFuncExtra1(GL, glDrawTexivOES, (coords), ((void*)coords, 5 * sizeof(GLfixed)))
DLLEXPORT void DLLEXPORTENTRY specialized_glDrawTexfOES (GLfloat x, GLfloat y, GLfloat z, GLfloat width, GLfloat height) DrawFunc(GL, glDrawTexfOES, (x, y, z, width, height))
DLLEXPORT void DLLEXPORTENTRY specialized_glDrawTexfvOES (const GLfloat *coords)
   DrawFuncExtra1(GL, glDrawTexfvOES, (coords), ((void*)coords, 5 * sizeof(GLfloat)))

DLLEXPORT void DLLEXPORTENTRY specialized_glDeleteRenderbuffersOES (GLsizei n, const GLuint* renderbuffers)
   FuncExtra1(GL, glDeleteRenderbuffersOES, (n, renderbuffers), ((void*)renderbuffers, n * sizeof(GLuint)))
DLLEXPORT void DLLEXPORTENTRY specialized_glBindFramebufferOES (GLenum target, GLuint framebuffer) DebugModeOnlyFunc(GL, glBindFramebufferOES, (target, framebuffer))
DLLEXPORT void DLLEXPORTENTRY specialized_glDeleteFramebuffersOES (GLsizei n, const GLuint* framebuffers)
   FuncExtra1(GL, glDeleteFramebuffersOES, (n, framebuffers), ((void*)framebuffers, n * sizeof(GLuint)))

DLLEXPORT void DLLEXPORTENTRY specialized_glDiscardFramebufferEXT (GLenum target, GLsizei numAttachments, const GLenum *attachments)
   FuncExtra1(GL, glDiscardFramebufferEXT, (target, numAttachments, attachments), ((void*)attachments, numAttachments * sizeof(GLenum)))
DLLEXPORT void DLLEXPORTENTRY specialized_glInsertEventMarkerEXT(GLsizei length, const GLchar *marker) DebugModeOnlyFunc(GL, glInsertEventMarkerEXT, (length, marker))
DLLEXPORT void DLLEXPORTENTRY specialized_glPushGroupMarkerEXT(GLsizei length, const GLchar *marker) DebugModeOnlyFunc(GL, glPushGroupMarkerEXT, (length, marker))
DLLEXPORT void DLLEXPORTENTRY specialized_glPopGroupMarkerEXT(void) DebugModeOnlyFunc(GL, glPopGroupMarkerEXT, ())

DLLEXPORT void DLLEXPORTENTRY specialized_glPointSizePointerOES (GLenum type, GLsizei stride, const GLvoid *pointer)
{
   APIInitAndLock locker(PL_FUNCTION);
   if (WantCommand(cmd_glPointSizePointerOES))
   {
      Packet   p(eAPI_FUNCTION);
      sCurPacket = &p;
      p.AddItem(cmd_glPointSizePointerOES);
      Packetize(type, stride, pointer);

      if (sCaptureStream)
      {
         GLint v[4];
         Real(glGetIntegerv)(GL_ARRAY_BUFFER_BINDING, v);
         Packetize(v[0] != 0);
      }

      bool ret = PostPacketize(&p);
      if (ret)
      {
         TIMESTAMP start, end;
         plGetTime(&start);
         Real(glPointSizePointerOES)(type, stride, pointer);
         plGetTime(&end);
         LogGLError();
         RetVoidFunc(plTimeDiffNano(&start, &end));
      }
   }
   else
      RealApiEvent(glPointSizePointerOES, (type, stride, pointer));
}

DLLEXPORT void DLLEXPORTENTRY specialized_glCompileShader(GLuint shader)
{
   APIInitAndLock __locker(PL_FUNCTION);
   if (WantCommand(cmd_glCompileShader))
   {
      Packet   __p(eAPI_FUNCTION);
      sCurPacket = &__p;
      __p.AddItem(cmd_glCompileShader);
      Packetize(shader);
      bool ret = PostPacketize(&__p);
      if (ret)
      {
         TIMESTAMP __start, __end;
         plGetTime(&__start);
         Real(glCompileShader)(shader);
         plGetTime(&__end);
         LogGLError();

         Packet   p(eRET_CODE);
         p.AddItem(2);

         // Custom extra ret data for compile status
         GLint val = 0;
         char buf[512];
         GLsizei len = 0;
         GLboolean valid = Real(glIsShader(shader));
         if (valid)
         {
            Real(glGetShaderiv)(shader, GL_COMPILE_STATUS, &val);
            Real(glGetShaderInfoLog)(shader, 511, &len, buf);
         }
         buf[len] = '\0';

         p.AddItem(valid);
         p.AddItem(val);
         p.AddItem(PacketItem(buf));

         p.AddItem(plTimeDiffNano(&__start, &__end));

         PostPacketize(&p);
      }
   }
   else
      RealApiEvent(glCompileShader, (shader));
}

DLLEXPORT void DLLEXPORTENTRY specialized_glLinkProgram(GLuint program)
{
   APIInitAndLock __locker(PL_FUNCTION);
   if (WantCommand(cmd_glLinkProgram))
   {
      Packet   __p(eAPI_FUNCTION);
      sCurPacket = &__p;
      __p.AddItem(cmd_glLinkProgram);
      Packetize(program);
      bool ret = PostPacketize(&__p);
      if (ret)
      {
         std::vector<std::string> names;

         TIMESTAMP __start, __end;
         plGetTime(&__start);
         Real(glLinkProgram)(program);
         plGetTime(&__end);
         LogGLError();

         Packet   p(eRET_CODE);
         p.AddItem(2);

         // Custom extra ret data for link status
         GLint val = 0;
         char buf[512];
         GLsizei len = 0;
         GLboolean valid = Real(glIsProgram(program));
         if (valid)
         {
            Real(glGetProgramiv)(program, GL_LINK_STATUS, &val);
            Real(glGetProgramInfoLog)(program, 511, &len, buf);
         }
         buf[len] = '\0';

         p.AddItem(valid);
         p.AddItem(val);
         p.AddItem(PacketItem(buf));

         if (sCaptureStream)
         {
            // Query program interface data and store in the capture
            GLint activeBlocks, maxBlockLen;
            Real(glGetProgramiv)(program, GL_ACTIVE_UNIFORM_BLOCKS, &activeBlocks);
            Real(glGetProgramiv)(program, GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH, &maxBlockLen);

            GLsizei bufSize = maxBlockLen + 1;
            GLchar *name = new GLchar[bufSize];
            name[bufSize - 1] = '\0';

            uint32_t version = 1;
            p.AddItem(version);
            p.AddItem(activeBlocks);
            p.AddItem(bufSize);

            // Query each uniform block
            for (GLint i = 0; i < activeBlocks; i++)
            {
               GLsizei len;

               Real(glGetActiveUniformBlockName)(program, i, bufSize, &len, name);

               names.push_back(name);
               p.AddItem(names.back().c_str());
            }

            delete[] name;
         }

         p.AddItem(plTimeDiffNano(&__start, &__end));

         PostPacketize(&p);
      }
   }
   else
      RealApiEvent(glLinkProgram, (program));
}

/* EGL */
DLLEXPORT EGLint EGLAPIENTRY specialized_eglGetError(void)
{
   APIInitAndLock locker(PL_FUNCTION);
   if (WantCommand(cmd_eglGetError))
   {
      Packet   p(eAPI_FUNCTION);
      sCurPacket = &p;
      p.AddItem(cmd_eglGetError);
      Packetize();
      bool ret = PostPacketize(&p);
      sSendNextRet = true;
      if (ret)
      {
         uint32_t tid = plGetThreadID();

         TIMESTAMP start, end;
         plGetTime(&start);
         EGLint err = sEGLErrors[tid].Get(); // Get the first stored error
         sEGLErrors[tid] = ErrorEGL();       // Clear the stored error
         // Make a call to get timing data, we know there won't be an error
         Real(eglGetError)();
         plGetTime(&end);
         RetFunc(plTimeDiffNano(&start, &end), (err));
         return err;
      }
      else
         return 0;
   }
   else
      RetRealApiEvent(EGLint, eglGetError, ());
}
DLLEXPORT EGLBoolean EGLAPIENTRY specialized_eglTerminate(EGLDisplay dpy)
{
   APIInitAndLock locker(PL_FUNCTION);
   if (sIgnoreEGLTerminate)
      return EGL_TRUE;

   if (WantCommand(cmd_eglTerminate))
   {
      Packet   p(eAPI_FUNCTION);
      sCurPacket = &p;
      p.AddItem(cmd_eglTerminate);
      Packetize(dpy);
      bool ret = PostPacketize(&p);
      if (ret)
      {
         TIMESTAMP start, end;
         plGetTime(&start);
         EGLBoolean r = Real(eglTerminate)(dpy);
         plGetTime(&end);
         LogEGLError();
         RetFunc(plTimeDiffNano(&start, &end), (r));
         return r;
      }
      else
         return 0;
   }
   else
      RetRealApiEvent(EGLBoolean, eglTerminate, (dpy));
}

DLLEXPORT EGLBoolean EGLAPIENTRY specialized_eglGetConfigs(EGLDisplay dpy, EGLConfig *configs, EGLint config_size, EGLint *num_config)
   FuncExtra1Ret(EGL, EGLBoolean, eglGetConfigs, (dpy, configs, config_size, num_config),
                             (configs, config_size * sizeof(EGLConfig)))

DLLEXPORT EGLBoolean EGLAPIENTRY specialized_eglChooseConfig(EGLDisplay dpy, const EGLint *attrib_list, EGLConfig *configs, EGLint config_size, EGLint *num_config)
   FuncExtra2Ret(EGL, EGLBoolean, eglChooseConfig, (dpy, attrib_list, configs, config_size, num_config),
                               ((void*)attrib_list, AttribListSize(attrib_list)),
                               (configs, config_size * sizeof(EGLConfig)))
DLLEXPORT EGLSurface EGLAPIENTRY specialized_eglCreateWindowSurface(EGLDisplay dpy, EGLConfig config, EGLNativeWindowType win, const EGLint *attrib_list)
   FuncExtra1RetSurface(EGL, EGLSurface, eglCreateWindowSurface, (dpy, config, win, attrib_list),
                                      ((void*)attrib_list, AttribListSize(attrib_list)))
DLLEXPORT EGLSurface EGLAPIENTRY specialized_eglCreatePbufferSurface(EGLDisplay dpy, EGLConfig config, const EGLint *attrib_list)
   FuncExtra1RetSurface(EGL, EGLSurface, eglCreatePbufferSurface, (dpy, config, attrib_list), ((void*)attrib_list, AttribListSize(attrib_list)))
DLLEXPORT EGLSurface EGLAPIENTRY specialized_eglCreatePixmapSurface(EGLDisplay dpy, EGLConfig config, EGLNativePixmapType pixmap, const EGLint *attrib_list)
   FuncExtra1RetSurface(EGL, EGLSurface, eglCreatePixmapSurface, (dpy, config, pixmap, attrib_list), ((void*)attrib_list, AttribListSize(attrib_list)))
DLLEXPORT EGLBoolean EGLAPIENTRY specialized_eglBindAPI(EGLenum api) DebugModeOnlyFuncRet(EGL, EGLBoolean, eglBindAPI, (api))
DLLEXPORT EGLBoolean EGLAPIENTRY specialized_eglWaitClient(void) DebugModeOnlyFuncRet(EGL, EGLBoolean, eglWaitClient, ())
DLLEXPORT EGLSurface EGLAPIENTRY specialized_eglCreatePbufferFromClientBuffer(EGLDisplay dpy, EGLenum buftype, EGLClientBuffer buffer, EGLConfig config, const EGLint *attrib_list)
   FuncExtra1RetSurface(EGL, EGLSurface, eglCreatePbufferFromClientBuffer, (dpy, buftype, buffer, config, attrib_list),
                                                ((void*)attrib_list, AttribListSize(attrib_list)))
DLLEXPORT EGLContext EGLAPIENTRY specialized_eglCreateContext(EGLDisplay dpy, EGLConfig config, EGLContext share_context, const EGLint *attrib_list)
{
   APIInitAndLock locker(PL_FUNCTION);
   if (WantCommand(cmd_eglCreateContext))
   {
      Packet   p(eAPI_FUNCTION);

      sCurPacket = &p;
      p.AddItem(cmd_eglCreateContext);
      Packetize(dpy, config, share_context, attrib_list);

      const EGLint *att_list = attrib_list;
      EGLint value = 1;

      while (att_list && (*att_list != EGL_NONE))
      {
         if (*att_list++ == EGL_CONTEXT_CLIENT_VERSION)
            value = *att_list;
         att_list++;
      }
      sCurPacket->AddItem(value);
      sCurPacket->AddItem(PacketItem((void*)attrib_list, AttribListSize(attrib_list)));

      // Extract config data and attach to packet
      EGLint   r = 0, g = 0, b = 0, a = 0, depthSize = 0, stencilSize = 0, samples = 0;

      Real(eglGetConfigAttrib)(dpy, config, EGL_DEPTH_SIZE, &depthSize);
      Real(eglGetConfigAttrib)(dpy, config, EGL_STENCIL_SIZE, &stencilSize);
      Real(eglGetConfigAttrib)(dpy, config, EGL_RED_SIZE, &r);
      Real(eglGetConfigAttrib)(dpy, config, EGL_GREEN_SIZE, &g);
      Real(eglGetConfigAttrib)(dpy, config, EGL_BLUE_SIZE, &b);
      Real(eglGetConfigAttrib)(dpy, config, EGL_ALPHA_SIZE, &a);
      Real(eglGetConfigAttrib)(dpy, config, EGL_SAMPLES, &samples);

      sCurPacket->AddItem(depthSize);
      sCurPacket->AddItem(stencilSize);
      sCurPacket->AddItem(r);
      sCurPacket->AddItem(g);
      sCurPacket->AddItem(b);
      sCurPacket->AddItem(a);
      sCurPacket->AddItem(samples);

      bool ret = PostPacketize(&p);
      if (ret)
      {
         TIMESTAMP start, end;
         plGetTime(&start);
         EGLContext c = Real(eglCreateContext)(dpy, config, share_context, attrib_list);
         plGetTime(&end);
         LogEGLError();
         RetFunc(plTimeDiffNano(&start, &end), (c));
         return c;
      }
      else
         return 0;
   }
   else
      RetRealApiEvent(EGLContext, eglCreateContext, (dpy, config, share_context, attrib_list));
}
DLLEXPORT EGLBoolean EGLAPIENTRY specialized_eglMakeCurrent(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx)
{
   APIInitAndLock locker(PL_FUNCTION);
   if (WantCommand(cmd_eglMakeCurrent))
   {
      Packet    p(eAPI_FUNCTION);
      EGLint    configId = 1;
      EGLConfig config = (EGLConfig)NULL;

      sCurPacket = &p;
      p.AddItem(cmd_eglMakeCurrent);
      Packetize(dpy, draw, read, ctx);

      EGLint   r = 0, g = 0, b = 0, a = 0, depthSize = 0, stencilSize = 0, samples = 0;

      if (ctx != EGL_NO_CONTEXT)
      {
         // Extract config data and attach to packet
         // Failure leaves configId unmodified (which is 1)
         if (Real(eglQueryContext)(dpy, ctx, EGL_CONFIG_ID, (EGLint*)&configId) == EGL_FALSE)
            ALOGD("******************* Unable to get a sensible return from eglQueryContext\n");

         config = ConfigIDToConfig(dpy, configId);

         Real(eglGetConfigAttrib)(dpy, config, EGL_DEPTH_SIZE, &depthSize);
         Real(eglGetConfigAttrib)(dpy, config, EGL_STENCIL_SIZE, &stencilSize);
         Real(eglGetConfigAttrib)(dpy, config, EGL_RED_SIZE, &r);
         Real(eglGetConfigAttrib)(dpy, config, EGL_GREEN_SIZE, &g);
         Real(eglGetConfigAttrib)(dpy, config, EGL_BLUE_SIZE, &b);
         Real(eglGetConfigAttrib)(dpy, config, EGL_ALPHA_SIZE, &a);
         Real(eglGetConfigAttrib)(dpy, config, EGL_SAMPLES, &samples);
      }

      // NOTE: MUST start with depth & stencil for backwards compatibility with older captures
      sCurPacket->AddItem(depthSize);
      sCurPacket->AddItem(stencilSize);
      sCurPacket->AddItem(r);
      sCurPacket->AddItem(g);
      sCurPacket->AddItem(b);
      sCurPacket->AddItem(a);
      sCurPacket->AddItem(samples);
      sCurPacket->AddItem(config);

      bool ret = PostPacketize(&p);
      sSendNextRet = true;
      if (ret)
      {
         TIMESTAMP start, end;
         plGetTime(&start);
         EGLBoolean b = Real(eglMakeCurrent)(dpy, draw, read, ctx);
         plGetTime(&end);
         LogEGLError();
         RetFunc(plTimeDiffNano(&start, &end), (b));
         return b;
      }
      else
         return 0;
   }
   else if (!sOrphaned && sBottleneckMode == Control::eTinyViewport)
   {
      EGLBoolean b = Real(eglMakeCurrent)(dpy, draw, read, ctx);
      Real(glViewport(0, 0, 1, 1));
      return b;
   }
   else
      RetRealApiEvent(EGLBoolean, eglMakeCurrent, (dpy, draw, read, ctx));
}
DLLEXPORT EGLSurface EGLAPIENTRY specialized_eglGetCurrentSurface(EGLint readdraw) FuncRetSurface(EGL, EGLSurface, eglGetCurrentSurface, (readdraw))
DLLEXPORT EGLBoolean EGLAPIENTRY specialized_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface)
{
   APIInitAndLock locker(PL_FUNCTION);
   if (!sOrphaned)
   {
      sFrameCnt++;
      sTotalFrameCnt++;

      // Poll the event timeline from the driver & kernel. Note: this does not send the event
      // data to GPUMonitor.
      CollectLowerLevelEventData();

      Packet   p(eAPI_FUNCTION);
      sCurPacket = &p;
      p.AddItem(cmd_eglSwapBuffers);
      Packetize(dpy, surface);
      bool ret = PostPacketize(&p);
      sSendNextRet = true;
      if (ret)
      {
         TIMESTAMP start, end;
         plGetTime(&start);
         RealApiEventRet(r, EGLBoolean, eglSwapBuffers, (dpy, surface));
         plGetTime(&end);
         LogEGLError();
         RetFunc(plTimeDiffNano(&start, &end), (r));
         return r;
      }
      else
         return 0;
   }
   else
      RetRealApiEvent(EGLBoolean, eglSwapBuffers, (dpy, surface));
}
DLLEXPORT EGLImageKHR EGLAPIENTRY specialized_eglCreateImageKHR(EGLDisplay dpy, EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLint *attrib_list)
   FuncExtra1Ret(EGL, EGLImageKHR, eglCreateImageKHR, (dpy, ctx, target, buffer, attrib_list), ((void*)attrib_list, AttribListSize(attrib_list)))

#if EGL_KHR_fence_sync
DLLEXPORT EGLSyncKHR EGLAPIENTRY specialized_eglCreateSyncKHR(EGLDisplay dpy, EGLenum type, const EGLint *attrib_list)
   FuncExtra1Ret(EGL, EGLSyncKHR, eglCreateSyncKHR, (dpy, type, attrib_list), ((void*)attrib_list, AttribListSize(attrib_list)))
DLLEXPORT EGLint EGLAPIENTRY specialized_eglClientWaitSyncKHR(EGLDisplay dpy, EGLSyncKHR sync, EGLint flags, EGLTimeKHR timeout)
{
   APIInitAndLock locker(PL_FUNCTION);
   if (WantCommand(cmd_eglClientWaitSyncKHR))
   {
      Packet   p(eAPI_FUNCTION);
      sCurPacket = &p;
      p.AddItem(cmd_eglClientWaitSyncKHR);
      Packetize(dpy, sync, flags, (uint32_t)timeout);
      bool ret = PostPacketize(&p);
      if (ret)
      {
         TIMESTAMP start, end;
         plGetTime(&start);
         EGLint r = Real(eglClientWaitSyncKHR)(dpy, sync, flags, timeout);
         plGetTime(&end);
         LogEGLError();
         RetFunc(plTimeDiffNano(&start, &end), (r));
         return r;
      }
      else
         return 0;
   }
   else
      RetRealApiEvent(EGLint, eglClientWaitSyncKHR, (dpy, sync, flags, timeout));
}
#endif

/* ES3 specifics */
DLLEXPORT void DLLEXPORTENTRY specialized_glDrawRangeElements (GLenum mode, GLuint s, GLuint e, GLsizei count, GLenum type, const GLvoid* indices)
{
   APIInitAndLock locker(PL_FUNCTION);
   if (WantCommand(cmd_glDrawRangeElements))
   {
      Packet   p(eAPI_FUNCTION);
      sCurPacket = &p;
      p.AddItem(cmd_glDrawRangeElements);
      Packetize(mode, s, e, count, type, indices);

      if (sCaptureStream)
      {
         GLint elementBind;
         Real(glGetIntegerv)(GL_ELEMENT_ARRAY_BUFFER_BINDING, &elementBind);

         Packetize(elementBind != 0);

         // Do we need to send the client-side indices array?
         if (elementBind == 0)
         {
            uint32_t bytesPerIndex = 0;
            switch (type)
            {
            case GL_UNSIGNED_BYTE  : bytesPerIndex = 1; break;
            case GL_UNSIGNED_SHORT : bytesPerIndex = 2; break;
            case GL_UNSIGNED_INT   : bytesPerIndex = 4; break;
            }

            sCurPacket->AddItem(PacketItem((void*)indices, bytesPerIndex * count));
         }
         else
            sCurPacket->AddItem(PacketItem((void*)NULL, 0));


         uint32_t esMajVer = GetESMajorVersion();
         Packetize(esMajVer);

         // Do we need to send vertex array data along with this packet
         {
            GLint typ, bytesPerVert, stride, norm;

            if (esMajVer >= 2)
            {
               GLint maxIndx = -1;

               GLint currentProg;
               GLint maxAttribs;

               Real(glGetIntegerv)(GL_CURRENT_PROGRAM, &currentProg);
               Real(glGetProgramiv)(currentProg, GL_ACTIVE_ATTRIBUTES, &maxAttribs);

               for (GLint a = 0; a < maxAttribs; a++)
               {
                  void   *attribs = NULL;
                  GLint   binding;
                  GLint   enabled;
                  GLint   location;
                  GLint   size;
                  GLenum  atype;
                  GLchar  name[1024];
                  GLsizei len = 1024;

                  name[0] = '\0';

                  Real(glGetIntegerv)(GL_CURRENT_PROGRAM, &currentProg);
                  Real(glGetActiveAttrib)(currentProg, a, len, &len, &size, &atype, name);
                  location = Real(glGetAttribLocation)(currentProg, name);

                  if (location != -1)
                  {
                     Real(glGetVertexAttribiv)(location, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, &binding);
                     Real(glGetVertexAttribiv)(location, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &enabled);

                     if (binding == 0 && enabled)
                     {
                        if (maxIndx == -1)
                        {
                           // Work out what the max index is (and therefore how many attribs to send)
                           maxIndx = FindMaxIndex(count, type, indices);

                           // Ensure we get the last element
                           maxIndx += 1;
                        }

                        Real(glGetVertexAttribPointerv)(location, GL_VERTEX_ATTRIB_ARRAY_POINTER, &attribs);
                        Real(glGetVertexAttribiv)(location, GL_VERTEX_ATTRIB_ARRAY_SIZE, &size);
                        Real(glGetVertexAttribiv)(location, GL_VERTEX_ATTRIB_ARRAY_TYPE, &typ);
                        Real(glGetVertexAttribiv)(location, GL_VERTEX_ATTRIB_ARRAY_NORMALIZED, &norm);
                        Real(glGetVertexAttribiv)(location, GL_VERTEX_ATTRIB_ARRAY_STRIDE, &stride);

                        if (stride != 0)
                           bytesPerVert = stride;
                        else
                           bytesPerVert = BytesForType(typ) * size;

                        if (attribs != NULL)
                        {
                           Packetize(location, size, typ, norm, stride);
                           sCurPacket->AddItem(PacketItem(attribs, bytesPerVert * maxIndx));
                        }
                     }
                  }
               }

               Packetize(-1);
            }
         }
      }

      bool ret = PostPacketize(&p);
      if (ret)
      {
         TIMESTAMP start, end;
         plGetTime(&start);
         Real(glDrawRangeElements)(mode, s, e, count, type, indices);
         plGetTime(&end);
         LogGLError();
         RetVoidFunc(plTimeDiffNano(&start, &end));
      }
   }
   else if (!sOrphaned && (sEnabled || (sMinimalModeCmds.find(cmd_glDrawRangeElements) != sMinimalModeCmds.end())) &&
            sBottleneckMode == Control::eNullDrawCalls)
   {
      // Do nothing
   }
   else
      RealApiEvent(glDrawRangeElements, (mode, s, e, count, type, indices));
}

DLLEXPORT void DLLEXPORTENTRY specialized_glTexImage3D (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid* pixels)
   TextureFuncExtra1MaybePBO(GL, glTexImage3D, (target, level, internalformat, width, height, depth, border, format, type, pixels),
                      pixels, TextureSize3D(width, height, depth, format, type, UnpackAlignment()), target, level, TinyTexImage3D)
DLLEXPORT void DLLEXPORTENTRY specialized_glTexSubImage3D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid* pixels)
   TextureFuncExtra1MaybePBO(GL, glTexSubImage3D, (target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels),
                      pixels, TextureSize3D(width, height, depth, format, type, UnpackAlignment()), target, level, TinyTexSubImage3D)
DLLEXPORT void DLLEXPORTENTRY specialized_glCompressedTexImage3D (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid* data)
   TextureFuncExtra1MaybePBO(GL, glCompressedTexImage3D, (target, level, internalformat, width, height, depth, border, imageSize, data),
                      data, imageSize, target, level, TinyTexImage3D)
DLLEXPORT void DLLEXPORTENTRY specialized_glCompressedTexSubImage3D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid* data)
   TextureFuncExtra1MaybePBO(GL, glCompressedTexSubImage3D, (target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data),
                      data, imageSize, target, level, TinyTexSubImage3D)
DLLEXPORT void DLLEXPORTENTRY specialized_glDeleteQueries (GLsizei n, const GLuint* ids)
   FuncExtra1(GL, glDeleteQueries, (n, ids), ((void*)ids, n * sizeof(GLuint)))

DLLEXPORT GLboolean DLLEXPORTENTRY specialized_glUnmapBuffer(GLenum target)
{
   APIInitAndLock locker(PL_FUNCTION);
   if (WantCommand(cmd_glUnmapBuffer))
   {
      Packet   p(eAPI_FUNCTION);
      sCurPacket = &p;
      p.AddItem(cmd_glUnmapBuffer);
      Packetize(target);

      if (sCaptureStream)
      {
         GLint access;
         Real(glGetBufferParameteriv)(target, GL_BUFFER_ACCESS_FLAGS, &access);
         if ((access & GL_MAP_WRITE_BIT) != 0)
         {
            GLvoid *ptr;
            GLint offset, len;

            Real(glGetBufferPointerv)(target, GL_BUFFER_MAP_POINTER, &ptr);
            Real(glGetBufferParameteriv)(target, GL_BUFFER_MAP_OFFSET, &offset);
            Real(glGetBufferParameteriv)(target, GL_BUFFER_MAP_LENGTH, &len);

            Packetize(true);
            Packetize(offset);
            Packetize(len);
            Packetize(PacketItem(ptr, len));
         }
         else
         {
            Packetize(false);
         }
      }

      bool ret = PostPacketize(&p);
      if (ret)
      {
         TIMESTAMP start, end;
         plGetTime(&start);
         GLboolean r = Real(glUnmapBuffer)(target);
         plGetTime(&end);
         LogGLError();
         RetFunc(plTimeDiffNano(&start, &end), (r));
         return r;
      }
      else
         return 0;
   }
   else
      RetRealApiEvent(GLboolean, glUnmapBuffer, (target));
}

DLLEXPORT void DLLEXPORTENTRY specialized_glDrawBuffers (GLsizei n, const GLenum* bufs)
   FuncExtra1(GL, glDrawBuffers, (n, bufs), ((void*)bufs, n * sizeof(GLenum)))
DLLEXPORT void DLLEXPORTENTRY specialized_glUniformMatrix2x3fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
   FuncExtra1(GL, glUniformMatrix2x3fv, (location, count, transpose, value), ((void*)value, count * 2 * 3 * sizeof(GLfloat)))
DLLEXPORT void DLLEXPORTENTRY specialized_glUniformMatrix3x2fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
   FuncExtra1(GL, glUniformMatrix3x2fv, (location, count, transpose, value), ((void*)value, count * 3 * 2 * sizeof(GLfloat)))
DLLEXPORT void DLLEXPORTENTRY specialized_glUniformMatrix2x4fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
   FuncExtra1(GL, glUniformMatrix2x4fv, (location, count, transpose, value), ((void*)value, count * 2 * 4 * sizeof(GLfloat)))
DLLEXPORT void DLLEXPORTENTRY specialized_glUniformMatrix4x2fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
   FuncExtra1(GL, glUniformMatrix4x2fv, (location, count, transpose, value), ((void*)value, count * 4 * 2 * sizeof(GLfloat)))
DLLEXPORT void DLLEXPORTENTRY specialized_glUniformMatrix3x4fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
   FuncExtra1(GL, glUniformMatrix3x4fv, (location, count, transpose, value), ((void*)value, count * 3 * 4 * sizeof(GLfloat)))
DLLEXPORT void DLLEXPORTENTRY specialized_glUniformMatrix4x3fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
   FuncExtra1(GL, glUniformMatrix4x3fv, (location, count, transpose, value), ((void*)value, count * 4 * 3 * sizeof(GLfloat)))

#if GL_OES_mapbuffer
DLLEXPORT GLboolean DLLEXPORTENTRY specialized_glUnmapBufferOES(GLenum target)
{
   APIInitAndLock locker(PL_FUNCTION);
   if (WantCommand(cmd_glUnmapBufferOES))
   {
      Packet   p(eAPI_FUNCTION);
      sCurPacket = &p;
      p.AddItem(cmd_glUnmapBufferOES);
      Packetize(target);

      if (sCaptureStream)
      {
         GLint access;
         Real(glGetBufferParameteriv)(target, GL_BUFFER_ACCESS_OES, &access);
         if ((access & GL_MAP_WRITE_BIT) != 0)
         {
            GLvoid *ptr;
            GLint len;

            Real(glGetBufferPointervOES)(target, GL_BUFFER_MAP_POINTER_OES, &ptr);
            Real(glGetBufferParameteriv)(target, GL_BUFFER_SIZE, &len);

            Packetize(true);
            Packetize(0);
            Packetize(len);
            Packetize(PacketItem(ptr, len));
         }
         else
         {
            Packetize(false);
         }
      }

      bool ret = PostPacketize(&p);
      if (ret)
      {
         TIMESTAMP start, end;
         plGetTime(&start);
         GLboolean r = Real(glUnmapBufferOES)(target);
         plGetTime(&end);
         LogGLError();
         RetFunc(plTimeDiffNano(&start, &end), (r));
         return r;
      }
      else
         return 0;
   }
   else
      RetRealApiEvent(GLboolean, glUnmapBufferOES, (target));
}
#endif /* GL_OES_mapbuffer */

DLLEXPORT void DLLEXPORTENTRY specialized_glFlushMappedBufferRange (GLenum target, GLintptr offset, GLsizeiptr length)
{
   APIInitAndLock locker(PL_FUNCTION);
   if (WantCommand(cmd_glFlushMappedBufferRange))
   {
      Packet   p(eAPI_FUNCTION);
      sCurPacket = &p;
      p.AddItem(cmd_glFlushMappedBufferRange);
      Packetize(target, offset, length);

      if (sCaptureStream)
      {
         GLint access;
         Real(glGetBufferParameteriv)(target, GL_BUFFER_ACCESS_FLAGS, &access);
         if ((access & GL_MAP_WRITE_BIT) != 0)
         {
            GLvoid *ptr;
            GLint mappedOffset, mappedLen;
            GLint sendLen;

            Real(glGetBufferPointerv)(target, GL_BUFFER_MAP_POINTER, &ptr);
            Real(glGetBufferParameteriv)(target, GL_BUFFER_MAP_OFFSET, &mappedOffset);
            Real(glGetBufferParameteriv)(target, GL_BUFFER_MAP_LENGTH, &mappedLen);

            sendLen = length;
            if (length + offset > mappedLen)
               sendLen = mappedLen - offset;

            Packetize(true);
            Packetize(mappedOffset);
            Packetize(mappedLen);
            Packetize(PacketItem((void*)((uintptr_t)ptr + offset), sendLen));
         }
         else
         {
            Packetize(false);
         }
      }

      bool ret = PostPacketize(&p);
      if (ret)
      {
         TIMESTAMP start, end;
         plGetTime(&start);
         Real(glFlushMappedBufferRange)(target, offset, length);
         plGetTime(&end);
         LogGLError();
         RetVoidFunc(plTimeDiffNano(&start, &end));
      }
   }
   else
      RealApiEvent(glFlushMappedBufferRange, (target, offset, length));
}

DLLEXPORT void DLLEXPORTENTRY specialized_glDeleteVertexArrays (GLsizei n, const GLuint* arrays)
   FuncExtra1(GL, glDeleteVertexArrays, (n, arrays), ((void*)arrays, n * sizeof(GLuint)))
DLLEXPORT void DLLEXPORTENTRY specialized_glTransformFeedbackVaryings (GLuint program, GLsizei count, const GLchar* const* varyings, GLenum bufferMode)
{
   APIInitAndLock locker(PL_FUNCTION);
   if (WantCommand(cmd_glTransformFeedbackVaryings))
   {
      Packet   p(eAPI_FUNCTION);
      sCurPacket = &p;
      p.AddItem(cmd_glTransformFeedbackVaryings);
      Packetize(program, count, varyings, bufferMode);

      if (sCaptureStream)
      {
         // Strings are NULL terminated
         for (int i = 0; i < count; i++)
            sCurPacket->AddItem(PacketItem((void*)varyings[i], strlen(varyings[i]) + 1));
      }

      bool ret = PostPacketize(&p);
      if (ret)
      {
         TIMESTAMP start, end;
         plGetTime(&start);
         Real(glTransformFeedbackVaryings)(program, count, varyings, bufferMode);
         plGetTime(&end);
         LogGLError();
         RetVoidFunc(plTimeDiffNano(&start, &end));
      }
   }
   else
      RealApiEvent(glTransformFeedbackVaryings, (program, count, varyings, bufferMode));
}

DLLEXPORT void DLLEXPORTENTRY specialized_glVertexAttribIPointer (GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid* pointer)
{
   APIInitAndLock locker(PL_FUNCTION);
   if (WantCommand(cmd_glVertexAttribIPointer))
   {
      Packet   p(eAPI_FUNCTION);
      sCurPacket = &p;
      p.AddItem(cmd_glVertexAttribIPointer);
      Packetize(index, size, type, stride, pointer);

      if (sCaptureStream)
      {
         GLint v[4];
         Real(glGetIntegerv)(GL_ARRAY_BUFFER_BINDING, v);
         Packetize(v[0] != 0);
      }

      bool ret = PostPacketize(&p);
      if (ret)
      {
         TIMESTAMP start, end;
         plGetTime(&start);
         Real(glVertexAttribIPointer)(index, size, type, stride, pointer);
         plGetTime(&end);
         LogGLError();
         RetVoidFunc(plTimeDiffNano(&start, &end));
      }
   }
   else
      RealApiEvent(glVertexAttribIPointer, (index, size, type, stride, pointer));
}

DLLEXPORT void DLLEXPORTENTRY specialized_glVertexAttribI4iv (GLuint index, const GLint* v)
   FuncExtra1(GL, glVertexAttribI4iv, (index, v), ((void*)v, 4 * sizeof(GLint)))
DLLEXPORT void DLLEXPORTENTRY specialized_glVertexAttribI4uiv (GLuint index, const GLuint* v)
   FuncExtra1(GL, glVertexAttribI4uiv, (index, v), ((void*)v, 4 * sizeof(GLuint)))
DLLEXPORT void DLLEXPORTENTRY specialized_glUniform1uiv (GLint location, GLsizei count, const GLuint* value)
   FuncExtra1(GL, glUniform1uiv, (location, count, value), ((void*)value, 1 * sizeof(GLuint)))
DLLEXPORT void DLLEXPORTENTRY specialized_glUniform2uiv (GLint location, GLsizei count, const GLuint* value)
   FuncExtra1(GL, glUniform2uiv, (location, count, value), ((void*)value, 2 * sizeof(GLuint)))
DLLEXPORT void DLLEXPORTENTRY specialized_glUniform3uiv (GLint location, GLsizei count, const GLuint* value)
   FuncExtra1(GL, glUniform3uiv, (location, count, value), ((void*)value, 3 * sizeof(GLuint)))
DLLEXPORT void DLLEXPORTENTRY specialized_glUniform4uiv (GLint location, GLsizei count, const GLuint* value)
   FuncExtra1(GL, glUniform4uiv, (location, count, value), ((void*)value, 4 * sizeof(GLuint)))
DLLEXPORT void DLLEXPORTENTRY specialized_glClearBufferiv (GLenum buffer, GLint drawbuffer, const GLint* value)
   FuncExtra1(GL, glClearBufferiv, (buffer, drawbuffer, value), ((void*)value, ((buffer == GL_DEPTH || buffer == GL_STENCIL) ? sizeof(GLint) : 4 * sizeof(GLint))))
DLLEXPORT void DLLEXPORTENTRY specialized_glClearBufferuiv (GLenum buffer, GLint drawbuffer, const GLuint* value)
   FuncExtra1(GL, glClearBufferuiv, (buffer, drawbuffer, value), ((void*)value, ((buffer == GL_DEPTH || buffer == GL_STENCIL) ? sizeof(GLuint) : 4 * sizeof(GLuint))))
DLLEXPORT void DLLEXPORTENTRY specialized_glClearBufferfv (GLenum buffer, GLint drawbuffer, const GLfloat* value)
   FuncExtra1(GL, glClearBufferfv, (buffer, drawbuffer, value), ((void*)value, ((buffer == GL_DEPTH || buffer == GL_STENCIL) ? sizeof(GLfloat) : 4 * sizeof(GLfloat))))
DLLEXPORT void DLLEXPORTENTRY specialized_glGetUniformIndices (GLuint program, GLsizei uniformCount, const GLchar* const* uniformNames, GLuint* uniformIndices)
{
   APIInitAndLock locker(PL_FUNCTION);
   if (WantCommand(cmd_glGetUniformIndices))
   {
      Packet   p(eAPI_FUNCTION);
      int32_t  i;

      sCurPacket = &p;
      p.AddItem(cmd_glGetUniformIndices);
      Packetize(program, uniformCount, uniformNames, uniformIndices);

      // Names are NULL terminated
      for (i = 0; i < uniformCount; i++)
         sCurPacket->AddItem(PacketItem((void*)uniformNames[i], strlen(uniformNames[i])));

      bool ret = PostPacketize(&p);
      if (ret)
      {
         TIMESTAMP start, end;
         plGetTime(&start);
         Real(glGetUniformIndices)(program, uniformCount, uniformNames, uniformIndices);
         plGetTime(&end);
         LogGLError();
         RetVoidFunc(plTimeDiffNano(&start, &end));
      }
   }
   else
      RealApiEvent(glGetUniformIndices, (program, uniformCount, uniformNames, uniformIndices));
}

DLLEXPORT void DLLEXPORTENTRY specialized_glGetActiveUniformsiv (GLuint program, GLsizei uniformCount, const GLuint* uniformIndices, GLenum pname, GLint* params)
   FuncExtra1(GL, glGetActiveUniformsiv, (program, uniformCount, uniformIndices, pname, params), ((void*)uniformIndices, uniformCount * sizeof(GLuint)))
DLLEXPORT void DLLEXPORTENTRY specialized_glDrawArraysInstanced (GLenum mode, GLint first, GLsizei count, GLsizei instanceCount)
{
   APIInitAndLock locker(PL_FUNCTION);
   if (WantCommand(cmd_glDrawArraysInstanced))
   {
      Packet   p(eAPI_FUNCTION);
      sCurPacket = &p;
      p.AddItem(cmd_glDrawArraysInstanced);
      Packetize(mode, first, count, instanceCount);

      if (sCaptureStream)
      {
         {
            GLint type, bytesPerVert, stride, norm;

            // Are we ES1 or ES2 or ES3?
            uint32_t esMajVer = GetESMajorVersion();
            Packetize(esMajVer);

            if (esMajVer >= 2)
            {
               GLint currentProg;
               GLint maxAttribs;

               Real(glGetIntegerv)(GL_CURRENT_PROGRAM, &currentProg);
               Real(glGetProgramiv)(currentProg, GL_ACTIVE_ATTRIBUTES, &maxAttribs);

               for (GLint a = 0; a < maxAttribs; a++)
               {
                  void   *attribs = NULL;
                  GLint   binding;
                  GLint   enabled;
                  GLint   location;
                  GLint   size;
                  GLenum  atype;
                  GLchar  name[1024];
                  GLsizei len = 1024;

                  name[0] = '\0';

                  Real(glGetActiveAttrib)(currentProg, a, len, &len, &size, &atype, name);
                  location = Real(glGetAttribLocation)(currentProg, name);

                  if (location != -1)
                  {
                     Real(glGetVertexAttribiv)(location, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, &binding);
                     Real(glGetVertexAttribiv)(location, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &enabled);

                     if (binding == 0 && enabled)
                     {
                        Real(glGetVertexAttribPointerv)(location, GL_VERTEX_ATTRIB_ARRAY_POINTER, &attribs);
                        Real(glGetVertexAttribiv)(location, GL_VERTEX_ATTRIB_ARRAY_SIZE, &size);
                        Real(glGetVertexAttribiv)(location, GL_VERTEX_ATTRIB_ARRAY_TYPE, &type);
                        Real(glGetVertexAttribiv)(location, GL_VERTEX_ATTRIB_ARRAY_NORMALIZED, &norm);
                        Real(glGetVertexAttribiv)(location, GL_VERTEX_ATTRIB_ARRAY_STRIDE, &stride);

                        if (stride != 0)
                           bytesPerVert = stride;
                        else
                           bytesPerVert = BytesForType(type) * size;

                        if (attribs != NULL)
                        {
                           Packetize(a, size, type, norm, stride);
                           sCurPacket->AddItem(PacketItem(attribs, bytesPerVert * (first + count)));
                        }
                     }
                  }
               }

               Packetize(-1);
            }
         }
      }

      bool ret = PostPacketize(&p);
      if (ret)
      {
         TIMESTAMP start, end;
         plGetTime(&start);
         Real(glDrawArraysInstanced)(mode, first, count, instanceCount);
         plGetTime(&end);
         LogGLError();
         RetVoidFunc(plTimeDiffNano(&start, &end));
      }
   }
   else if (!sOrphaned && (sEnabled || (sMinimalModeCmds.find(cmd_glDrawArraysInstanced) != sMinimalModeCmds.end())) &&
            sBottleneckMode == Control::eNullDrawCalls)
   {
      // Do nothing
   }
   else
      RealApiEvent(glDrawArraysInstanced, (mode, first, count, instanceCount));
}

DLLEXPORT void DLLEXPORTENTRY specialized_glDrawElementsInstanced (GLenum mode, GLsizei count, GLenum type, const GLvoid* indices, GLsizei instanceCount)
{
   APIInitAndLock locker(PL_FUNCTION);
   if (WantCommand(cmd_glDrawElementsInstanced))
   {
      Packet   p(eAPI_FUNCTION);
      sCurPacket = &p;
      p.AddItem(cmd_glDrawElementsInstanced);
      Packetize(mode, count, type, indices, instanceCount);

      if (sCaptureStream)
      {
         GLint elementBind;
         Real(glGetIntegerv)(GL_ELEMENT_ARRAY_BUFFER_BINDING, &elementBind);

         Packetize(elementBind != 0);

         // Do we need to send the client-side indices array?
         if (elementBind == 0)
         {
            uint32_t bytesPerIndex = 0;
            switch (type)
            {
            case GL_UNSIGNED_BYTE  : bytesPerIndex = 1; break;
            case GL_UNSIGNED_SHORT : bytesPerIndex = 2; break;
            case GL_UNSIGNED_INT   : bytesPerIndex = 4; break;
            }

            sCurPacket->AddItem(PacketItem((void*)indices, bytesPerIndex * count));
         }
         else
            sCurPacket->AddItem(PacketItem((void*)NULL, 0));


         uint32_t esMajVer = GetESMajorVersion();
         Packetize(esMajVer);

         // Do we need to send vertex array data along with this packet
         {
            GLint typ, bytesPerVert, stride, norm;

            if (esMajVer >= 2)
            {
               GLint maxIndx = -1;

               GLint currentProg;
               GLint maxAttribs;

               Real(glGetIntegerv)(GL_CURRENT_PROGRAM, &currentProg);
               Real(glGetProgramiv)(currentProg, GL_ACTIVE_ATTRIBUTES, &maxAttribs);

               for (GLint a = 0; a < maxAttribs; a++)
               {
                  void   *attribs = NULL;
                  GLint   binding;
                  GLint   enabled;
                  GLint   location;
                  GLint   size;
                  GLenum  atype;
                  GLchar  name[1024];
                  GLsizei len = 1024;

                  name[0] = '\0';

                  Real(glGetIntegerv)(GL_CURRENT_PROGRAM, &currentProg);
                  Real(glGetActiveAttrib)(currentProg, a, len, &len, &size, &atype, name);
                  location = Real(glGetAttribLocation)(currentProg, name);

                  if (location != -1)
                  {
                     Real(glGetVertexAttribiv)(location, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, &binding);
                     Real(glGetVertexAttribiv)(location, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &enabled);

                     if (binding == 0 && enabled)
                     {
                        if (maxIndx == -1)
                        {
                           // Work out what the max index is (and therefore how many attribs to send)
                           maxIndx = FindMaxIndex(count, type, indices);

                           // Ensure we get the last element
                           maxIndx += 1;
                        }

                        Real(glGetVertexAttribPointerv)(location, GL_VERTEX_ATTRIB_ARRAY_POINTER, &attribs);
                        Real(glGetVertexAttribiv)(location, GL_VERTEX_ATTRIB_ARRAY_SIZE, &size);
                        Real(glGetVertexAttribiv)(location, GL_VERTEX_ATTRIB_ARRAY_TYPE, &typ);
                        Real(glGetVertexAttribiv)(location, GL_VERTEX_ATTRIB_ARRAY_NORMALIZED, &norm);
                        Real(glGetVertexAttribiv)(location, GL_VERTEX_ATTRIB_ARRAY_STRIDE, &stride);

                        if (stride != 0)
                           bytesPerVert = stride;
                        else
                           bytesPerVert = BytesForType(typ) * size;

                        if (attribs != NULL)
                        {
                           Packetize(location, size, typ, norm, stride);
                           sCurPacket->AddItem(PacketItem(attribs, bytesPerVert * maxIndx));
                        }
                     }
                  }
               }

               Packetize(-1);
            }
         }
      }

      bool ret = PostPacketize(&p);
      if (ret)
      {
         TIMESTAMP start, end;
         plGetTime(&start);
         Real(glDrawElementsInstanced)(mode, count, type, indices, instanceCount);
         plGetTime(&end);
         LogGLError();
         RetVoidFunc(plTimeDiffNano(&start, &end));
      }
   }
   else if (!sOrphaned && (sEnabled || (sMinimalModeCmds.find(cmd_glDrawElementsInstanced) != sMinimalModeCmds.end())) &&
            sBottleneckMode == Control::eNullDrawCalls)
   {
      // Do nothing
   }
   else
      RealApiEvent(glDrawElementsInstanced, (mode, count, type, indices, instanceCount));
}

DLLEXPORT GLenum DLLEXPORTENTRY specialized_glClientWaitSync (GLsync sync, GLbitfield flags, GLuint64 timeout)
{
   APIInitAndLock locker(PL_FUNCTION);
   if (WantCommand(cmd_glClientWaitSync))
   {
      Packet   p(eAPI_FUNCTION);
      sCurPacket = &p;
      p.AddItem(cmd_glClientWaitSync);
      Packetize(sync, flags, (uint32_t)(timeout & 0xFFFFFFFF));
      Packetize((uint32_t)(timeout >> 32));
      bool ret = PostPacketize(&p);
      if (ret)
      {
         TIMESTAMP start, end;
         plGetTime(&start);
         GLenum r = Real(glClientWaitSync)(sync, flags, timeout);
         plGetTime(&end);
         LogGLError();
         RetFunc(plTimeDiffNano(&start, &end), (r));
         return r;
      }
      else
         return 0;
   }
   else
      RetRealApiEvent(GLenum, glClientWaitSync, (sync, flags, timeout));
}

DLLEXPORT void DLLEXPORTENTRY specialized_glWaitSync (GLsync sync, GLbitfield flags, GLuint64 timeout)
{
   APIInitAndLock locker(PL_FUNCTION);
   if (WantCommand(cmd_glWaitSync))
   {
      Packet   p(eAPI_FUNCTION);
      sCurPacket = &p;
      p.AddItem(cmd_glWaitSync);
      Packetize(sync, flags, (uint32_t)(timeout & 0xFFFFFFFF));
      Packetize((uint32_t)(timeout >> 32));
      bool ret = PostPacketize(&p);
      if (ret)
      {
         TIMESTAMP start, end;
         plGetTime(&start);
         Real(glWaitSync)(sync, flags, timeout);
         plGetTime(&end);
         LogGLError();
         RetVoidFunc(plTimeDiffNano(&start, &end));
      }
   }
   else
      RealApiEvent(glWaitSync, (sync, flags, timeout));
}

DLLEXPORT void DLLEXPORTENTRY specialized_glDeleteSamplers (GLsizei count, const GLuint* samplers)
   FuncExtra1(GL, glDeleteSamplers, (count, samplers), ((void*)samplers, count * sizeof(GLuint)))
DLLEXPORT void DLLEXPORTENTRY specialized_glSamplerParameteriv (GLuint sampler, GLenum pname, const GLint* param)
   FuncExtra1(GL, glSamplerParameteriv, (sampler, pname, param), ((void*)param, sizeof(GLint)))
DLLEXPORT void DLLEXPORTENTRY specialized_glSamplerParameterfv (GLuint sampler, GLenum pname, const GLfloat* param)
   FuncExtra1(GL, glSamplerParameterfv, (sampler, pname, param), ((void*)param, sizeof(GLfloat)))
DLLEXPORT void DLLEXPORTENTRY specialized_glDeleteTransformFeedbacks (GLsizei n, const GLuint* ids)
   FuncExtra1(GL, glDeleteTransformFeedbacks, (n, ids), ((void*)ids, n * sizeof(GLuint)))
DLLEXPORT void DLLEXPORTENTRY specialized_glProgramBinary (GLuint program, GLenum binaryFormat, const GLvoid* binary, GLsizei length)
   FuncExtra1(GL, glProgramBinary, (program, binaryFormat, binary, length), ((void*)binary, length))
DLLEXPORT void DLLEXPORTENTRY specialized_glInvalidateFramebuffer (GLenum target, GLsizei numAttachments, const GLenum* attachments)
   FuncExtra1(GL, glInvalidateFramebuffer, (target, numAttachments, attachments), ((void*)attachments, numAttachments * sizeof(GLenum)))
DLLEXPORT void DLLEXPORTENTRY specialized_glInvalidateSubFramebuffer (GLenum target, GLsizei numAttachments, const GLenum* attachments, GLint x, GLint y, GLsizei width, GLsizei height)
   FuncExtra1(GL, glInvalidateSubFramebuffer, (target, numAttachments, attachments, x, y, width, height), ((void*)attachments, numAttachments * sizeof(GLenum)))

// ES3.1
DLLEXPORT GLuint DLLEXPORTENTRY specialized_glCreateShaderProgramv(GLenum type, GLsizei count, const GLchar *const*strings)
{
   APIInitAndLock locker(PL_FUNCTION);
   if (WantCommand(cmd_glCreateShaderProgramv))
   {
      Packet   p(eAPI_FUNCTION);
      int32_t  i;

      sCurPacket = &p;
      p.AddItem(cmd_glCreateShaderProgramv);
      Packetize(type, count, strings);

      // Strings are NULL terminated
      for (i = 0; i < count; i++)
         sCurPacket->AddItem(PacketItem((void*)strings[i], strlen(strings[i])));

      bool ret = PostPacketize(&p);
      if (ret)
      {
         TIMESTAMP start, end;
         plGetTime(&start);
         GLuint r = Real(glCreateShaderProgramv)(type, count, strings);
         plGetTime(&end);
         LogGLError();

         Packet   p(eRET_CODE);
         p.AddItem(0);
         Packetize(r);

         // Custom extra ret data for link status
         GLint val = 0;
         char buf[512];
         GLsizei len = 0;
         GLboolean valid = Real(glIsProgram(r));
         if (valid)
         {
            Real(glGetProgramiv)(r, GL_LINK_STATUS, &val);
            Real(glGetProgramInfoLog)(r, 511, &len, buf);
         }
         buf[len] = '\0';

         p.AddItem(valid);
         p.AddItem(val);
         p.AddItem(PacketItem(buf));

         p.AddItem(plTimeDiffNano(&start, &end));

         PostPacketize(&p);

         return r;
      }
      return 0;
   }
   else
      RetRealApiEvent(GLuint, glCreateShaderProgramv, (type, count, strings));
}

DLLEXPORT void DLLEXPORTENTRY specialized_glDeleteProgramPipelines(GLsizei n, const GLuint *pipelines) FuncExtra1(GL, glDeleteProgramPipelines, (n, pipelines), ((void*)pipelines, n * sizeof(GLuint)))
DLLEXPORT void DLLEXPORTENTRY specialized_glProgramUniform1iv(GLuint program, GLint location, GLsizei count, const GLint *value) FuncExtra1(GL, glProgramUniform1iv, (program, location, count, value), ((void*)value, count * sizeof(GLint)))
DLLEXPORT void DLLEXPORTENTRY specialized_glProgramUniform2iv(GLuint program, GLint location, GLsizei count, const GLint *value) FuncExtra1(GL, glProgramUniform2iv, (program, location, count, value), ((void*)value, count * sizeof(GLint)))
DLLEXPORT void DLLEXPORTENTRY specialized_glProgramUniform3iv(GLuint program, GLint location, GLsizei count, const GLint *value) FuncExtra1(GL, glProgramUniform3iv, (program, location, count, value), ((void*)value, count * sizeof(GLint)))
DLLEXPORT void DLLEXPORTENTRY specialized_glProgramUniform4iv(GLuint program, GLint location, GLsizei count, const GLint *value) FuncExtra1(GL, glProgramUniform4iv, (program, location, count, value), ((void*)value, count * sizeof(GLint)))
DLLEXPORT void DLLEXPORTENTRY specialized_glProgramUniform1uiv(GLuint program, GLint location, GLsizei count, const GLuint *value) FuncExtra1(GL, glProgramUniform1uiv, (program, location, count, value), ((void*)value, count * sizeof(GLuint)))
DLLEXPORT void DLLEXPORTENTRY specialized_glProgramUniform2uiv(GLuint program, GLint location, GLsizei count, const GLuint *value) FuncExtra1(GL, glProgramUniform2uiv, (program, location, count, value), ((void*)value, count * sizeof(GLuint)))
DLLEXPORT void DLLEXPORTENTRY specialized_glProgramUniform3uiv(GLuint program, GLint location, GLsizei count, const GLuint *value) FuncExtra1(GL, glProgramUniform3uiv, (program, location, count, value), ((void*)value, count * sizeof(GLuint)))
DLLEXPORT void DLLEXPORTENTRY specialized_glProgramUniform4uiv(GLuint program, GLint location, GLsizei count, const GLuint *value) FuncExtra1(GL, glProgramUniform4uiv, (program, location, count, value), ((void*)value, count * sizeof(GLuint)))
DLLEXPORT void DLLEXPORTENTRY specialized_glProgramUniform1fv(GLuint program, GLint location, GLsizei count, const GLfloat *value) FuncExtra1(GL, glProgramUniform1fv,  (program, location, count, value), ((void*)value, count * sizeof(GLfloat)))
DLLEXPORT void DLLEXPORTENTRY specialized_glProgramUniform2fv(GLuint program, GLint location, GLsizei count, const GLfloat *value) FuncExtra1(GL, glProgramUniform2fv,  (program, location, count, value), ((void*)value, count * sizeof(GLfloat)))
DLLEXPORT void DLLEXPORTENTRY specialized_glProgramUniform3fv(GLuint program, GLint location, GLsizei count, const GLfloat *value) FuncExtra1(GL, glProgramUniform3fv,  (program, location, count, value), ((void*)value, count * sizeof(GLfloat)))
DLLEXPORT void DLLEXPORTENTRY specialized_glProgramUniform4fv(GLuint program, GLint location, GLsizei count, const GLfloat *value) FuncExtra1(GL, glProgramUniform4fv,  (program, location, count, value), ((void*)value, count * sizeof(GLfloat)))
DLLEXPORT void DLLEXPORTENTRY specialized_glProgramUniformMatrix2fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) FuncExtra1(GL, glProgramUniformMatrix2fv,     (program, location, count, transpose, value), ((void*)value, count * sizeof(GLfloat)))
DLLEXPORT void DLLEXPORTENTRY specialized_glProgramUniformMatrix3fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) FuncExtra1(GL, glProgramUniformMatrix3fv,     (program, location, count, transpose, value), ((void*)value, count * sizeof(GLfloat)))
DLLEXPORT void DLLEXPORTENTRY specialized_glProgramUniformMatrix4fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) FuncExtra1(GL, glProgramUniformMatrix4fv,     (program, location, count, transpose, value), ((void*)value, count * sizeof(GLfloat)))
DLLEXPORT void DLLEXPORTENTRY specialized_glProgramUniformMatrix2x3fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) FuncExtra1(GL, glProgramUniformMatrix2x3fv, (program, location, count, transpose, value), ((void*)value, count * sizeof(GLfloat)))
DLLEXPORT void DLLEXPORTENTRY specialized_glProgramUniformMatrix3x2fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) FuncExtra1(GL, glProgramUniformMatrix3x2fv, (program, location, count, transpose, value), ((void*)value, count * sizeof(GLfloat)))
DLLEXPORT void DLLEXPORTENTRY specialized_glProgramUniformMatrix2x4fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) FuncExtra1(GL, glProgramUniformMatrix2x4fv, (program, location, count, transpose, value), ((void*)value, count * sizeof(GLfloat)))
DLLEXPORT void DLLEXPORTENTRY specialized_glProgramUniformMatrix4x2fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) FuncExtra1(GL, glProgramUniformMatrix4x2fv, (program, location, count, transpose, value), ((void*)value, count * sizeof(GLfloat)))
DLLEXPORT void DLLEXPORTENTRY specialized_glProgramUniformMatrix3x4fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) FuncExtra1(GL, glProgramUniformMatrix3x4fv, (program, location, count, transpose, value), ((void*)value, count * sizeof(GLfloat)))
DLLEXPORT void DLLEXPORTENTRY specialized_glProgramUniformMatrix4x3fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) FuncExtra1(GL, glProgramUniformMatrix4x3fv, (program, location, count, transpose, value), ((void*)value, count * sizeof(GLfloat)))

#if GL_OES_vertex_array_object
DLLEXPORT void DLLEXPORTENTRY specialized_glDeleteVertexArraysOES(GLsizei n, const GLuint *arrays) FuncExtra1(GL, glDeleteVertexArraysOES, (n, arrays), ((void*)arrays, n * sizeof(GLuint)))
#endif

#if GL_KHR_debug
DLLEXPORT void DLLEXPORTENTRY specialized_glDebugMessageControlKHR(GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint *ids, GLboolean enabled)
   FuncExtra1(GL, glDebugMessageControlKHR, (source, type, severity, count, ids, enabled), ((void*)ids, count * sizeof(GLuint)))
DLLEXPORT void DLLEXPORTENTRY specialized_glDebugMessageCallbackKHR(GLDEBUGPROCKHR callback, const void *userParam)
{
   APIInitAndLock locker(PL_FUNCTION);
   if (WantCommand(cmd_glDebugMessageCallbackKHR))
   {
      Packet   p(eAPI_FUNCTION);
      sCurPacket = &p;
      p.AddItem(cmd_glDebugMessageCallbackKHR);
      Packetize((void*)callback, userParam);
      bool ret = PostPacketize(&p);
      if (ret)
      {
         TIMESTAMP start, end;
         plGetTime(&start);
         Real(glDebugMessageCallbackKHR)(callback, userParam);
         plGetTime(&end);
         LogGLError();
         RetVoidFunc(plTimeDiffNano(&start, &end));
      }
   }
   else
      RealApiEvent(glDebugMessageCallbackKHR, (callback, userParam));
}
#endif

DLLEXPORT __eglMustCastToProperFunctionPointerType DLLEXPORTENTRY specialized_eglGetProcAddress(const char *procname)
{
   APIInitAndLock locker(PL_FUNCTION);
   if (WantCommand(cmd_eglGetProcAddress))
   {
      Packet   p(eAPI_FUNCTION);
      sCurPacket = &p;
      p.AddItem(cmd_eglGetProcAddress);
      Packetize(procname);
      bool ret = PostPacketize(&p);
      if (ret)
      {
         TIMESTAMP start, end;
         plGetTime(&start);

         // Find the function in the default search paths
         void *func = dlsym(RTLD_DEFAULT, procname);

         __eglMustCastToProperFunctionPointerType r;
         if (func != NULL)
            r = (__eglMustCastToProperFunctionPointerType)func;
         else
            r = Real(eglGetProcAddress)(procname);

         plGetTime(&end);
         LogEGLError();
         RetFunc(plTimeDiffNano(&start, &end), ((void*)r));
         return r;
      }
      else
         return 0;
   }
   else
      RetRealApiEvent(__eglMustCastToProperFunctionPointerType, eglGetProcAddress, (procname));
}

} // extern "C"

#include "basefunctions.inc"

#define SPECIAL(name) map_##name = specialized_##name;

class CApiClassTable
{
public:
   CApiClassTable()
   {
#include "construction.inc"

      SPECIAL(eglGetProcAddress);
      SPECIAL(eglGetError);
      SPECIAL(eglTerminate);
      SPECIAL(eglGetConfigs);
      SPECIAL(eglChooseConfig);
      SPECIAL(eglCreateWindowSurface);
      SPECIAL(eglCreatePbufferSurface);
      SPECIAL(eglCreatePixmapSurface);
      SPECIAL(eglBindAPI);
      SPECIAL(eglWaitClient);
      SPECIAL(eglCreatePbufferFromClientBuffer);
      SPECIAL(eglCreateContext);
      SPECIAL(eglMakeCurrent);
      SPECIAL(eglGetCurrentSurface);
      SPECIAL(eglSwapBuffers);
#if EGL_KHR_image
      SPECIAL(eglCreateImageKHR);
#endif
#if EGL_KHR_fence_sync
      SPECIAL(eglCreateSyncKHR);
      SPECIAL(eglClientWaitSyncKHR);
#endif

      SPECIAL(glGetError);
      SPECIAL(glBindFramebuffer);
      SPECIAL(glBlendEquation);
      SPECIAL(glBlendEquationSeparate);
      SPECIAL(glBlendFunc);
      SPECIAL(glBlendFuncSeparate);
      SPECIAL(glBufferData);
      SPECIAL(glBufferSubData);
      SPECIAL(glClearColor);
      SPECIAL(glClipPlanef);
      SPECIAL(glClipPlanex);
      SPECIAL(glColorPointer);
      SPECIAL(glCompressedTexImage2D);
      SPECIAL(glCompressedTexSubImage2D);
      SPECIAL(glDeleteBuffers);
      SPECIAL(glDeleteFramebuffers);
      SPECIAL(glDeleteRenderbuffers);
      SPECIAL(glDeleteTextures);
      SPECIAL(glDisable);
      SPECIAL(glDrawArrays);
      SPECIAL(glDrawElements);
      SPECIAL(glFinish);
      SPECIAL(glFogfv);
      SPECIAL(glFogxv);
      SPECIAL(glLightModelfv);
      SPECIAL(glLightModelxv);
      SPECIAL(glLightfv);
      SPECIAL(glLightxv);
      SPECIAL(glLoadMatrixf);
      SPECIAL(glLoadMatrixx);
      SPECIAL(glMaterialfv);
      SPECIAL(glMaterialxv);
      SPECIAL(glMultMatrixf);
      SPECIAL(glMultMatrixx);
      SPECIAL(glNormalPointer);
      SPECIAL(glPointParameterfv);
      SPECIAL(glPointParameterxv);
      SPECIAL(glShaderBinary);
      SPECIAL(glShaderSource);
      SPECIAL(glTexCoordPointer);
      SPECIAL(glTexEnvfv);
      SPECIAL(glTexEnviv);
      SPECIAL(glTexEnvxv);
      SPECIAL(glTexImage2D);
      SPECIAL(glTexParameterfv);
      SPECIAL(glTexParameteriv);
      SPECIAL(glTexParameterxv);
      SPECIAL(glTexSubImage2D);
      SPECIAL(glUniform1fv);
      SPECIAL(glUniform1i);
      SPECIAL(glUniform1iv);
      SPECIAL(glUniform2fv);
      SPECIAL(glUniform2iv);
      SPECIAL(glUniform3fv);
      SPECIAL(glUniform3iv);
      SPECIAL(glUniform4fv);
      SPECIAL(glUniform4iv);
      SPECIAL(glUniformMatrix2fv);
      SPECIAL(glUniformMatrix3fv);
      SPECIAL(glUniformMatrix4fv);
      SPECIAL(glVertexAttrib1fv);
      SPECIAL(glVertexAttrib2fv);
      SPECIAL(glVertexAttrib3fv);
      SPECIAL(glVertexAttrib4fv);
      SPECIAL(glVertexAttribPointer);
      SPECIAL(glVertexPointer);
      SPECIAL(glViewport);
      SPECIAL(glDrawRangeElements);
      SPECIAL(glTexImage3D);
      SPECIAL(glTexSubImage3D);
      SPECIAL(glCompressedTexImage3D);
      SPECIAL(glCompressedTexSubImage3D);
      SPECIAL(glDeleteQueries);
      SPECIAL(glUnmapBuffer);
      SPECIAL(glDrawBuffers);
      SPECIAL(glUniformMatrix2x3fv);
      SPECIAL(glUniformMatrix3x2fv);
      SPECIAL(glUniformMatrix2x4fv);
      SPECIAL(glUniformMatrix4x2fv);
      SPECIAL(glUniformMatrix3x4fv);
      SPECIAL(glUniformMatrix4x3fv);
      SPECIAL(glFlushMappedBufferRange);
      SPECIAL(glDeleteVertexArrays);
      SPECIAL(glTransformFeedbackVaryings);
      SPECIAL(glVertexAttribIPointer);
      SPECIAL(glVertexAttribI4iv);
      SPECIAL(glVertexAttribI4uiv);
      SPECIAL(glUniform1uiv);
      SPECIAL(glUniform2uiv);
      SPECIAL(glUniform3uiv);
      SPECIAL(glUniform4uiv);
      SPECIAL(glClearBufferiv);
      SPECIAL(glClearBufferuiv);
      SPECIAL(glClearBufferfv);
      SPECIAL(glGetUniformIndices);
      SPECIAL(glGetActiveUniformsiv);
      SPECIAL(glDrawArraysInstanced);
      SPECIAL(glDrawElementsInstanced);
      SPECIAL(glClientWaitSync);
      SPECIAL(glWaitSync);
      SPECIAL(glDeleteSamplers);
      SPECIAL(glSamplerParameteriv);
      SPECIAL(glSamplerParameterfv);
      SPECIAL(glDeleteTransformFeedbacks);
      SPECIAL(glProgramBinary);
      SPECIAL(glInvalidateFramebuffer);
      SPECIAL(glInvalidateSubFramebuffer);
      SPECIAL(glCreateShaderProgramv);
      SPECIAL(glDeleteProgramPipelines);
      SPECIAL(glProgramUniform1iv);
      SPECIAL(glProgramUniform2iv);
      SPECIAL(glProgramUniform3iv);
      SPECIAL(glProgramUniform4iv);
      SPECIAL(glProgramUniform1uiv);
      SPECIAL(glProgramUniform2uiv);
      SPECIAL(glProgramUniform3uiv);
      SPECIAL(glProgramUniform4uiv);
      SPECIAL(glProgramUniform1fv);
      SPECIAL(glProgramUniform2fv);
      SPECIAL(glProgramUniform3fv);
      SPECIAL(glProgramUniform4fv);
      SPECIAL(glProgramUniformMatrix2fv);
      SPECIAL(glProgramUniformMatrix3fv);
      SPECIAL(glProgramUniformMatrix4fv);
      SPECIAL(glProgramUniformMatrix2x3fv);
      SPECIAL(glProgramUniformMatrix3x2fv);
      SPECIAL(glProgramUniformMatrix2x4fv);
      SPECIAL(glProgramUniformMatrix4x2fv);
      SPECIAL(glProgramUniformMatrix3x4fv);
      SPECIAL(glProgramUniformMatrix4x3fv);
      SPECIAL(glCompileShader);
      SPECIAL(glLinkProgram);
#if GL_OES_draw_texture
      SPECIAL(glDrawTexsOES);
      SPECIAL(glDrawTexiOES);
      SPECIAL(glDrawTexxOES);
      SPECIAL(glDrawTexsvOES);
      SPECIAL(glDrawTexivOES);
      SPECIAL(glDrawTexxvOES);
      SPECIAL(glDrawTexfOES);
      SPECIAL(glDrawTexfvOES);
#endif
#if GL_OES_mapbuffer
      SPECIAL(glUnmapBufferOES);
#endif
#if GL_OES_framebuffer_object
      SPECIAL(glDeleteRenderbuffersOES);
      SPECIAL(glBindFramebufferOES);
      SPECIAL(glDeleteFramebuffersOES);
#endif
#if GL_EXT_discard_framebuffer
      SPECIAL(glDiscardFramebufferEXT);
#endif
#if GL_EXT_debug_marker
      SPECIAL(glInsertEventMarkerEXT);
      SPECIAL(glPushGroupMarkerEXT);
      SPECIAL(glPopGroupMarkerEXT);
#endif
#if GL_OES_point_size_array
      SPECIAL(glPointSizePointerOES);
#endif
#if GL_OES_vertex_array_object
      SPECIAL(glDeleteVertexArraysOES);
#endif
#if GL_KHR_debug
      SPECIAL(glDebugMessageControlKHR);
      SPECIAL(glDebugMessageCallbackKHR);
#endif
   }

public:
#include "members.inc"
};

static CApiClassTable sAPIClassTable;

extern "C" {
#include "apifuncs.inc"


#ifdef BCG_ABSTRACT_PLATFORM
/* These entry points are only available with BCG's runtime loadable platforms */
/* NOTE: These aren't strictly necessary when using LD_PRELOAD to manage the intercept
   since it will look for these in the original khronos dll anyway and therefore
   will implicitly bypass the gpumon_hooks. I've left them in place in case someone
   needs to replace the khronos dll with a renamed gpumon_hook.dll rather than use
   LD_PRELOAD.
*/
struct BEGL_SchedInterface;
struct BEGL_DisplayInterface;
struct BEGL_MemoryInterface;

DLLEXPORT void DLLEXPORTENTRY BEGL_RegisterSchedInterface(BEGL_SchedInterface *iface)
{
   // Pass through
   APIInitAndLock locker(PL_FUNCTION, false);
   Real(BEGL_RegisterSchedInterface)(iface);
}

DLLEXPORT void DLLEXPORTENTRY BEGL_RegisterDisplayInterface(BEGL_DisplayInterface *iface)
{
   // Pass through
   APIInitAndLock locker(PL_FUNCTION, false);
   Real(BEGL_RegisterDisplayInterface)(iface);
}

DLLEXPORT void DLLEXPORTENTRY BEGL_RegisterMemoryInterface(BEGL_MemoryInterface *iface)
{
   // Pass through
   APIInitAndLock locker(PL_FUNCTION, false);
   Real(BEGL_RegisterMemoryInterface)(iface);
}

DLLEXPORT void DLLEXPORTENTRY BEGL_PlatformAboutToShutdown(void)
{
   // Pass through
   APIInitAndLock locker(PL_FUNCTION, false);
   Real(BEGL_PlatformAboutToShutdown)();
}

#endif

#ifdef ANDROID
/* These functions are defined in nexus_egl_client.cpp (part of libnexuseglclient.so) */
void* EGL_nexus_join(char *client_process_name);
void EGL_nexus_unjoin(void *nexus_client);

DLLEXPORT void DLLEXPORTENTRY dummyForLinkage(void)
{
   // Force linkage of these
   EGL_nexus_join(NULL);
   EGL_nexus_unjoin(NULL);
}

#endif
}
