/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/

#include <sstream>

#include "Command.h"
#include "spytool_replay.h"
#include "GLIncludes.h"
#include "DeviceCaps.h"

#define GetB(type, param)     ((type)m_packet.Item(param + 1).GetBoolean())
#define GetI8(type, param)    ((type)m_packet.Item(param + 1).GetInt8())
#define GetI16(type, param)   ((type)m_packet.Item(param + 1).GetInt16())
#define GetI32(type, param)   ((type)m_packet.Item(param + 1).GetInt32())
#define GetU8(type, param)    ((type)m_packet.Item(param + 1).GetUInt8())
#define GetU16(type, param)   ((type)m_packet.Item(param + 1).GetUInt16())
#define GetU32(type, param)   ((type)m_packet.Item(param + 1).GetUInt32())
#define GetPtr(type, param)   ((type)m_packet.Item(param + 1).GetVoidPtr())
#define GetF(type, param)     ((type)m_packet.Item(param + 1).GetFloat())
#define GetVP(type, param)    ((type)m_packet.Item(param + 1).GetVoidPtr())
#define GetCP(type, param)    ((type)m_packet.Item(param + 1).GetCharPtr())
#define GetArray(param, ptr)  (m_packet.Item(param + 1).GetArray((uint8_t**)&ptr))
#define GetArrayPtr(type, param)    ((type)m_packet.Item(param + 1).GetArrayPtr())

#define GetCurProgram()                   (m_perContextState[m_curContext].m_curProgram)
#define GetEGLSurface(type, param)        ((type)m_replay->MapSurface(m_packet.Item(param + 1).GetUInt32()))
#define GetEGLDisplay(type, param)        ((type)m_replay->MapDisplay(m_packet.Item(param + 1).GetUInt32()))
#define GetEGLContext(type, param)        ((type)m_replay->MapContext(m_packet.Item(param + 1).GetUInt32()))
#define GetEGLConfig(type, param)         ((type)m_replay->MapConfig(m_packet.Item(param + 1).GetUInt32()))
#define GetEGLClientBuffer(type, param)   ((type)m_replay->MapClientBuffer(m_packet.Item(param + 1).GetUInt32()))
#define GetEGLImage(type, param)          ((type)m_replay->MapEGLImage((GLuint)m_packet.Item(param + 1).GetUInt32()))
#define GetEGLSync(type, param)           ((type)m_replay->MapEGLSync((GLuint)m_packet.Item(param + 1).GetUInt32()))
#define GetSync(type, param)              ((type)m_replay->MapSync((GLuint)m_packet.Item(param + 1).GetUInt32()))
#define GetShader(type, param)            ((type)m_replay->MapShader((GLuint)m_packet.Item(param + 1).GetUInt32()))
#define GetProgram(type, param)           ((type)m_replay->MapProgram((GLuint)m_packet.Item(param + 1).GetUInt32()))
#define GetUniform(param, program)        (m_replay->MapUniform((GLint)m_packet.Item(param + 1).GetInt32(), GetProgram(GLuint, program)))
#define GetCurUniform(param)              (m_replay->MapUniform((GLint)m_packet.Item(param + 1).GetInt32(), GetCurProgram()))
#define GetLocation(param, program)       (m_replay->MapLocation((GLint)m_packet.Item(param + 1).GetInt32(), GetProgram(GLuint, program)))
#define GetUniformBlockIndex(param, program)(m_replay->MapUniformBlockIndex((GLint)m_packet.Item(param + 1).GetInt32(), GetProgram(GLuint, program)))
#define GetCurLocation(param)             (m_replay->MapLocation((GLint)m_packet.Item(param + 1).GetInt32(), GetCurProgram()))
#define GetQuery(type, param)             ((type)m_replay->MapQuery((GLuint)m_packet.Item(param + 1).GetUInt32()))
#define GetVertexArray(type, param)       ((type)m_replay->MapVertexArray((GLuint)m_packet.Item(param + 1).GetUInt32()))
#define GetSampler(type, param)           ((type)m_replay->MapSampler((GLuint)m_packet.Item(param + 1).GetUInt32()))
#define GetTF(type, param)                ((type)m_replay->MapTF((GLuint)m_packet.Item(param + 1).GetUInt32()))
#define GetProgramPipeline(type, param)   ((type)m_replay->MapProgramPipeline((GLuint)m_packet.Item(param + 1).GetUInt32()))

static void CheckError()
{
#ifdef EMULATED
   GLenum glErr = glGetError();
   if (glErr != GL_NO_ERROR)
   {
      switch (glErr)
      {
      case GL_INVALID_ENUM :                    printf("Error: GL_INVALID_ENUM\n"); break;
      case GL_INVALID_FRAMEBUFFER_OPERATION :   printf("Error: GL_INVALID_FRAMEBUFFER_OPERATION\n"); break;
      case GL_INVALID_VALUE :                   printf("Error: GL_INVALID_VALUE\n"); break;
      case GL_INVALID_OPERATION :               printf("Error: GL_INVALID_OPERATION\n"); break;
      case GL_OUT_OF_MEMORY :                   printf("Error: GL_OUT_OF_MEMORY\n"); break;
      default :                                 printf("Error: Unknown GL %d\n", glErr); break;
      }
   }

   EGLint eglErr = eglGetError();
   if (eglErr != EGL_SUCCESS)
   {
      switch (eglErr)
      {
      case EGL_NOT_INITIALIZED      : printf("Error: EGL_NOT_INITIALIZED\n"); break;
      case EGL_BAD_ACCESS           : printf("Error: EGL_BAD_ACCESS\n"); break;
      case EGL_BAD_ALLOC            : printf("Error: EGL_BAD_ALLOC\n"); break;
      case EGL_BAD_ATTRIBUTE        : printf("Error: EGL_BAD_ATTRIBUTE\n"); break;
      case EGL_BAD_CONFIG           : printf("Error: EGL_BAD_CONFIG\n"); break;
      case EGL_BAD_CONTEXT          : printf("Error: EGL_BAD_CONTEXT\n"); break;
      case EGL_BAD_CURRENT_SURFACE  : printf("Error: EGL_BAD_CURRENT_SURFACE\n"); break;
      case EGL_BAD_DISPLAY          : printf("Error: EGL_BAD_DISPLAY\n"); break;
      case EGL_BAD_MATCH            : printf("Error: EGL_BAD_MATCH\n"); break;
      case EGL_BAD_NATIVE_PIXMAP    : printf("Error: EGL_BAD_NATIVE_PIXMAP\n"); break;
      case EGL_BAD_NATIVE_WINDOW    : printf("Error: EGL_BAD_NATIVE_WINDOW\n"); break;
      case EGL_BAD_PARAMETER        : printf("Error: EGL_BAD_PARAMETER\n"); break;
      case EGL_BAD_SURFACE          : printf("Error: EGL_BAD_SURFACE\n"); break;
      case EGL_CONTEXT_LOST         : printf("Error: EGL_CONTEXT_LOST\n"); break;
      default                       : printf("Error: Unknown EGL %d\n", eglErr); break;
      }
   }
#endif
}

// Static state that is preserved between commands
EGLContext                                Command::m_curContext = 0;
std::map<EGLContext, PerContextState>     Command::m_perContextState;
uint32_t                                  Command::m_curFrame = 1;
EGLSurface                                Command::m_curDrawSurface = EGL_NO_SURFACE;
EGLContext                                Command::m_delayedContextContext = 0;
EGLDisplay                                Command::m_delayedContextDisplay = 0;
EGLConfig                                 Command::m_delayedContextConfig = 0;
EGLContext                                Command::m_delayedContextShare = 0;
uint32_t                                  Command::m_delayedContextESVer = 2;

EGLint                                    Command::dummyEGLint[256];
EGLenum                                   Command::dummyEGLenum[256];
GLuint                                    Command::dummyGLuint[256];
GLint                                     Command::dummyGLint[256];
GLsizei                                   Command::dummyGLsizei[256];
#if GL_ES_VERSION_3_0
GLint64                                   Command::dummyGLint64[256];
#endif
GLfloat                                   Command::dummyGLfloat[256];
GLfixed                                   Command::dummyGLfixed[256];
GLenum                                    Command::dummyGLenum[256];
GLboolean                                 Command::dummyGLboolean[256];
uint8_t                                   Command::dummyvoid[256];
void                                      *Command::dummyvoidptr[256];
EGLConfig                                 Command::dummyEGLConfig[1024];
EGLSurface                                Command::dummyEGLSurface[256];
EGLBoolean                                Command::dummyEGLBoolean[256];
char                                      Command::dummychar[128 * 1024];
#if EGL_BRCM_performance_counters
EGLuint64BRCM                             Command::dummyEGLuint64BRCM[256];
#endif
#if EGL_VERSION_1_5
EGLAttrib                                 Command::dummyEGLAttrib[256];
#endif
void                                      *Command::array[256];
GLchar                                    Command::dummyGLchar[128 * 1024];

std::string                               Command::m_extensions;

Command::Command()
{
   Clear();
}

void Command::Clear()
{
   m_packet = Packet();
   m_retPacket = Packet();
   m_byteSize = 0;

   uint32_t i;
   for (i = 0; i < m_deleteList.size(); i++)
      delete [] m_deleteList[i];

   m_deleteList.clear();
}

Command::~Command()
{
   Clear();
}

void Command::GetGLExtensions()
{
   if (m_extensions == "")
   {
      const char *exts = (const char *)glGetString(GL_EXTENSIONS);
      if (exts != NULL)
         m_extensions = std::string(exts);
   }
}

void Command::WarnNotAvailable(const char *func)
{
   printf("WARNING: Trying to replay a command (%s) that is not available\n", func);
}

void Command::WarnDataNotHandled(const char *func)
{
   printf("WARNING: %s has extra packetized data that is not being handled.\n"
          "You may need to add a specialization for it.\n", func);
}

EGLConfig Command::MatchConfig(uint32_t r, uint32_t g, uint32_t b, uint32_t a, uint32_t d, uint32_t s, uint32_t samples)
{
   EGLint     confAttribs[256];
   int        i = 0;
   int        numConfigs = 0;

   EGLDisplay disp = eglGetDisplay(EGL_DEFAULT_DISPLAY);
   if (m_delayedContextDisplay == 0)
      m_delayedContextDisplay = disp;

   if (r > 8 || g > 8 || b > 8 || a > 8 || d > 32 || s > 8 || samples > 4)
   {
      printf("******************      WARNING      ************************\n");
      printf("MatchConfig given invalid parameters - using a default config\n");
      printf("*************************************************************\n");

      r = std::min(r, 8u);
      g = std::min(g, 8u);
      b = std::min(b, 8u);
      a = std::min(a, 8u);
      d = std::min(d, 24u);
      s = std::min(s, 8u);
      samples = std::min(samples, 4u);
   }

   confAttribs[i++] = EGL_RED_SIZE;
   confAttribs[i++] = r;
   confAttribs[i++] = EGL_GREEN_SIZE;
   confAttribs[i++] = g;
   confAttribs[i++] = EGL_BLUE_SIZE;
   confAttribs[i++] = b;
   confAttribs[i++] = EGL_ALPHA_SIZE;
   confAttribs[i++] = a;
   confAttribs[i++] = EGL_DEPTH_SIZE;
   confAttribs[i++] = d;
   confAttribs[i++] = EGL_STENCIL_SIZE;
   confAttribs[i++] = s;

   if (samples > 1)
   {
      confAttribs[i++] = EGL_SAMPLE_BUFFERS;
      confAttribs[i++] = 1;
      confAttribs[i++] = EGL_SAMPLES;
      confAttribs[i++] = samples;
   }

   confAttribs[i++] = EGL_SURFACE_TYPE;
   confAttribs[i++] = EGL_WINDOW_BIT;
   confAttribs[i++] = EGL_RENDERABLE_TYPE;
   confAttribs[i++] = EGL_OPENGL_ES2_BIT;
   confAttribs[i++] = EGL_NONE;

   if (!eglGetConfigs(disp, NULL, 0, &numConfigs))
      throw("eglGetConfigs() failed");

   std::vector<EGLConfig> eglConfig(numConfigs);

   if ((numConfigs == 0) || !eglChooseConfig(disp, confAttribs, &eglConfig[0], numConfigs, &numConfigs))
      throw("eglChooseConfig() failed");

   int configToUse = 0;

   for (i = 0; i < numConfigs; i++)
   {
      /*
         From the EGL spec - 3.4.1
         This rule places configs with deeper color buffers first in the list returned by eglChooseConfig.
         Applications may find this counterintuitive, and need to perform additional processing on the list of
         configs to find one best matching their requirements. For example, specifying RGBA depths of 5651
         could return a list whose first config has a depth of 8888.
      */
      EGLint red_size, green_size, blue_size, alpha_size, depth_size;
      eglGetConfigAttrib(disp, eglConfig[i], EGL_RED_SIZE, &red_size);
      eglGetConfigAttrib(disp, eglConfig[i], EGL_GREEN_SIZE, &green_size);
      eglGetConfigAttrib(disp, eglConfig[i], EGL_BLUE_SIZE, &blue_size);
      eglGetConfigAttrib(disp, eglConfig[i], EGL_ALPHA_SIZE, &alpha_size);
      eglGetConfigAttrib(disp, eglConfig[i], EGL_DEPTH_SIZE, &depth_size);

      if (r == (uint32_t)red_size && g == (uint32_t)green_size && b == (uint32_t)blue_size && a == (uint32_t)alpha_size)
      {
         // Exact match - take it
         configToUse = i;
         break;
      }
   }

   return eglConfig[configToUse];

}

// For older capture files
EGLConfig Command::MatchConfig(uint32_t r, uint32_t g, uint32_t b, uint32_t a)
{
   return MatchConfig(r, g, b, a, 24, 8, 1);
}

// Overridden egl call
EGLBoolean Command::eglSwapInterval(EGLDisplay dpy, EGLint interval)
{
   // Always take this apps swap interval, not the captured one.
   return ::eglSwapInterval(dpy, m_replay->GetOptions().GetSwapInterval());
}

// Overridden egl call
EGLBoolean Command::eglMakeCurrent(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx)
{
   EGLBoolean ret = ::eglMakeCurrent(dpy, draw, read, ctx);

   m_curDrawSurface = draw;
   m_curContext = ctx;

   if (draw != EGL_NO_SURFACE)
   {
      // Ensure we set the swap interval we want
      ::eglSwapInterval(dpy, m_replay->GetOptions().GetSwapInterval());

      // Override the viewport if required
      if (m_replay->HasWindowSizeMapping(draw))
      {
         const WindowSize &vp = m_replay->MapWindowSize(draw);

         uint32_t x = 0, y = 0;
         uint32_t w = vp.m_w;
         uint32_t h = vp.m_h;
         GLint    p;
         glGetIntegerv(GL_FRAMEBUFFER_BINDING, &p);
         if (p == 0)
            m_replay->RescaleViewport(draw, &x, &y, &w, &h);

         glViewport(x, y, w, h);
      }
   }

   return ret;
}

bool Command::Execute(SpyToolReplay *replay, bool timing)
{
   m_replay = replay;

   switch (m_packet.Type())
   {
   case eREINIT :
      // The client has torn down and restarted, so must we
      replay->GetPlatform().RestartPlatformDisplay();
      return false;
   case eTHREAD_CHANGE :
      // Fake a makeCurrent
      {
         EGLDisplay disp = eglGetCurrentDisplay();
         if (disp == EGL_NO_DISPLAY)
            disp = eglGetDisplay(EGL_DEFAULT_DISPLAY);

         eglMakeCurrent(disp,
                     (replay->MapSurface(m_packet.Item(2).GetUInt32())),
                     (replay->MapSurface(m_packet.Item(3).GetUInt32())),
                     (replay->MapContext(m_packet.Item(1).GetUInt32())));
      }
      return false;
   case eAPI_FUNCTION :
   case eRET_CODE :
      break;      // Don't return
   default :
      return false;
   }

   eGLCommand cmd = m_packet.Item(0).GetFunc();

   timing = timing && !replay->SkipFrame(m_curFrame);
   bsg::HighResTime start, end;
   if (timing)
      start = bsg::HighResTime::Now();

   switch (cmd)
   {
#include "CommandCases.inc"

   default: printf("MISSING CASE IN %s AT LINE %d\n", __FILE__, __LINE__);
   }

   if (timing)
   {
      end = bsg::HighResTime::Now();
      bsg::HighResTime fromSOF = start - replay->FrameStartTime();
      bsg::HighResTime cmdCost = end - start;
      replay->AddTiming(cmd, fromSOF.Microseconds(), cmdCost.Microseconds());
   }

   CheckError();

   return cmd == cmd_eglSwapBuffers;
}


void Command::ProcessDeferredVAPs()
{
   std::vector<DeferredVertexAttribPointer> *deferredVAPs;
   GLint                                     savedBuffer, buffer;

   deferredVAPs = &m_perContextState[m_curContext].m_deferredVAPs;

   if (deferredVAPs->size() > 0)
   {
      glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &savedBuffer);
      buffer = savedBuffer;

      for (uint32_t d = 0; d < deferredVAPs->size(); d++)
      {
         DeferredVertexAttribPointer &dvap((*deferredVAPs)[d]);
         if (buffer != (GLint)dvap.m_boundBuffer)
         {
            buffer = dvap.m_boundBuffer;
            glBindBuffer(GL_ARRAY_BUFFER, buffer);
         }

         if (dvap.m_iptr)
         {
#if GL_ES_VERSION_3_0
            glVertexAttribIPointer(m_replay->MapLocation(dvap.m_index, dvap.m_curProgram), dvap.m_size, dvap.m_type,
                                   dvap.m_stride, dvap.m_ptr);
#endif
         }
         else
         {
            glVertexAttribPointer(m_replay->MapLocation(dvap.m_index, dvap.m_curProgram), dvap.m_size, dvap.m_type,
                                  dvap.m_norm, dvap.m_stride, dvap.m_ptr);
         }
      }

      if (buffer != savedBuffer)
         glBindBuffer(GL_ARRAY_BUFFER, savedBuffer);

      deferredVAPs->clear();
   }
}

// For some reason, there is no typedef for this in any of the GL headers!
typedef void (*PFNGLPOINTSIZEPOINTEROESPROC)(GLenum type, GLsizei stride, const GLvoid *pointer);

#include "CommandMethods.inc"

#define SPECIAL(f) void SpecializedCommand::run_##f()

SPECIAL(glCreateProgram) { m_replay->AddProgramMapping(m_retPacket.Item(1).GetUInt32(), glCreateProgram()); }
SPECIAL(glCreateShader)  { m_replay->AddShaderMapping(m_retPacket.Item(1).GetUInt32(), glCreateShader(GetU32(GLenum, 0))); }
SPECIAL(glGetAttribLocation) { m_replay->AddLocationMapping(m_retPacket.Item(1).GetInt32(), glGetAttribLocation(GetProgram(GLuint, 0), GetCP(const GLchar*, 1)), GetProgram(GLuint, 0)); }
SPECIAL(glGetUniformLocation) { m_replay->AddUniformMapping(m_retPacket.Item(1).GetInt32(), glGetUniformLocation(GetProgram(GLuint, 0), GetCP(const GLchar*, 1)), GetProgram(GLuint, 0)); }
SPECIAL(glBindAttribLocation) { glBindAttribLocation(GetProgram(GLuint, 0), GetLocation(1, 0), GetCP(const GLchar*, 2)); }
SPECIAL(glClear)
{
   if (!m_replay->SkipFrame(m_curFrame))
      glClear(GetU32(GLbitfield, 0));
}
SPECIAL(glColorPointer)
{
   GLboolean b = GetB(bool, 4); // True if this is for a VBO - i.e. last arg is actually an index into the VBO
   if (b)
      glColorPointer(GetI32(GLint, 0), GetU32(GLenum, 1), GetI32(GLsizei, 2), GetPtr(GLvoid *, 3));
   else
   {
      // We do nothing for client side data - we'll handle this in glDraw*
   }
}
SPECIAL(glCompressedTexImage2D)
{
   dummyGLint[0] = GetArray(8, array[0]);
   if (dummyGLint[0] == 0 && array[0] == NULL && m_packet.NumItems() > 10)
      array[0] = GetPtr(GLvoid*, 9);
   glCompressedTexImage2D(GetU32(GLenum, 0), GetI32(GLint, 1), GetU32(GLenum, 2), GetI32(GLsizei, 3), GetI32(GLsizei, 4), GetI32(GLint, 5), GetI32(GLsizei, 6), array[0]);
}
SPECIAL(glCompressedTexSubImage2D)
{
   dummyGLint[0] = GetArray(9, array[0]);
   if (dummyGLint[0] == 0 && array[0] == NULL && m_packet.NumItems() > 11)
      array[0] = GetPtr(GLvoid*, 10);
   glCompressedTexSubImage2D(GetU32(GLenum, 0), GetI32(GLint, 1), GetI32(GLint, 2), GetI32(GLint, 3), GetI32(GLsizei, 4), GetI32(GLsizei, 5), GetU32(GLenum, 6), GetI32(GLsizei, 7), array[0]);
}
SPECIAL(glDisableVertexAttribArray) { glDisableVertexAttribArray(GetCurLocation(0)); }
SPECIAL(glDrawArrays)
{
   if (m_replay->SkipFrame(m_curFrame))
      return;

   if (m_replay->IncrSkipDrawCall())
      return;

   {
      // Get ES1 or 2 or 3
      uint32_t esVer = GetU32(uint32_t, 3);

      // We need to get any client side data from the packet now and install it
      if (esVer == 1)
      {
#ifndef EMULATED
         GLint indx = -1;
         GLint i = 5;
         GLint a = 0;
         GLint size;
         GLint type;
         GLint stride;

         if (!GetB(bool, 4))
         {
            GLint curTU;

            glGetIntegerv(GL_CLIENT_ACTIVE_TEXTURE, &curTU);

            do
            {
               indx = GetI32(GLint, i); i++;

               if (indx != -1)
               {
                  size = GetI32(GLint, i); i++;
                  type = GetI32(GLint, i); i++;
                  stride = GetI32(GLint, i); i++;
                  GetArray(i, array[a]); i++;

                  switch (indx)
                  {
                  case GL_VERTEX_ARRAY_POINTER :             glVertexPointer(size, type, stride, array[a]); break;
                  case GL_COLOR_ARRAY_POINTER :              glColorPointer(size, type, stride, array[a]); break;
                  case GL_TEXTURE_COORD_ARRAY_POINTER + 0 :  glClientActiveTexture(GL_TEXTURE0 + 0);
                                                               glTexCoordPointer(size, type, stride, array[a]); break;
                  case GL_TEXTURE_COORD_ARRAY_POINTER + 1 :  glClientActiveTexture(GL_TEXTURE0 + 1);
                                                               glTexCoordPointer(size, type, stride, array[a]); break;
                  case GL_TEXTURE_COORD_ARRAY_POINTER + 2 :  glClientActiveTexture(GL_TEXTURE0 + 2);
                                                               glTexCoordPointer(size, type, stride, array[a]); break;
                  case GL_TEXTURE_COORD_ARRAY_POINTER + 3 :  glClientActiveTexture(GL_TEXTURE0 + 3);
                                                               glTexCoordPointer(size, type, stride, array[a]); break;
                  case GL_TEXTURE_COORD_ARRAY_POINTER + 4 :  glClientActiveTexture(GL_TEXTURE0 + 4);
                                                               glTexCoordPointer(size, type, stride, array[a]); break;
                  case GL_TEXTURE_COORD_ARRAY_POINTER + 5 :  glClientActiveTexture(GL_TEXTURE0 + 5);
                                                               glTexCoordPointer(size, type, stride, array[a]); break;
                  case GL_TEXTURE_COORD_ARRAY_POINTER + 6 :  glClientActiveTexture(GL_TEXTURE0 + 6);
                                                               glTexCoordPointer(size, type, stride, array[a]); break;
                  case GL_TEXTURE_COORD_ARRAY_POINTER + 7 :  glClientActiveTexture(GL_TEXTURE0 + 7);
                                                               glTexCoordPointer(size, type, stride, array[a]); break;
                  case GL_NORMAL_ARRAY_POINTER :             glNormalPointer(type, stride, array[a]); break;
                  case GL_POINT_SIZE_ARRAY_POINTER_OES :     glPointSizePointerOES(type, stride, array[a]); break;
                  }
                  a++;
               }
            }
            while (indx != -1);

            glClientActiveTexture(curTU);
         }
#else
         printf("EMULATOR DOES NOT SUPPORT ES1 CODE\n");
#endif
      }
      else if (esVer >= 2)
      {
         GLint indx = -1;
         GLint i = 4;
         GLint a = 0;

         do
         {
            indx = GetI32(GLint, i); i++;
            if (indx != -1)
            {
               GLint size = GetI32(GLint, i); i++;
               GLint type = GetI32(GLint, i); i++;
               GLint norm = GetI32(GLint, i); i++;
               GLint stride = GetI32(GLint, i); i++;
               GetArray(i, array[a]); i++;
               glVertexAttribPointer(m_replay->MapLocation(indx, GetCurProgram()), size, type, norm, stride, array[a]);
               a++;
            }
         }
         while (indx != -1);

         ProcessDeferredVAPs();
      }
   }

   glDrawArrays(GetU32(GLenum, 0), GetI32(GLint, 1), GetI32(GLsizei, 2));
}

SPECIAL(glDrawElements)
{
   if (m_replay->SkipFrame(m_curFrame))
      return;

   if (m_replay->IncrSkipDrawCall())
      return;

   bool indicesBound = GetB(bool, 4);

   if (!indicesBound)
      GetArray(5, array[0]);

   // Get ES1 or 2 or 3
   uint32_t esVer = GetU32(uint32_t, 6);

   if (esVer == 1)
   {
#ifndef EMULATED
      bool attribsBound = GetB(bool, 7);

      if (attribsBound && indicesBound)
         glDrawElements(GetU32(GLenum, 0), GetI32(GLsizei, 1), GetU32(GLenum, 2), GetPtr(GLvoid*, 3));
      else if (attribsBound && !indicesBound)
         glDrawElements(GetU32(GLenum, 0), GetI32(GLsizei, 1), GetU32(GLenum, 2), (GLvoid*)array[0]);
      else if (!attribsBound)
      {
         // We need to get the client side attribs from the packet now and install them
         GLint indx = -1;
         GLint i = 8;
         GLint a = 1;
         GLint size;
         GLint type;
         GLint stride;
         GLint curTU;

         glGetIntegerv(GL_CLIENT_ACTIVE_TEXTURE, &curTU);

         do
         {
            indx = GetI32(GLint, i); i++;

            if (indx != -1)
            {
               size = GetI32(GLint, i); i++;
               type = GetI32(GLint, i); i++;
               stride = GetI32(GLint, i); i++;
               GetArray(i, array[a]); i++;

               switch (indx)
               {
               case GL_VERTEX_ARRAY_POINTER :             glVertexPointer(size, type, stride, array[a]); break;
               case GL_COLOR_ARRAY_POINTER :              glColorPointer(size, type, stride, array[a]); break;
               case GL_TEXTURE_COORD_ARRAY_POINTER + 0 :  glClientActiveTexture(GL_TEXTURE0 + 0);
                                                            glTexCoordPointer(size, type, stride, array[a]); break;
               case GL_TEXTURE_COORD_ARRAY_POINTER + 1 :  glClientActiveTexture(GL_TEXTURE0 + 1);
                                                            glTexCoordPointer(size, type, stride, array[a]); break;
               case GL_TEXTURE_COORD_ARRAY_POINTER + 2 :  glClientActiveTexture(GL_TEXTURE0 + 2);
                                                            glTexCoordPointer(size, type, stride, array[a]); break;
               case GL_TEXTURE_COORD_ARRAY_POINTER + 3 :  glClientActiveTexture(GL_TEXTURE0 + 3);
                                                            glTexCoordPointer(size, type, stride, array[a]); break;
               case GL_TEXTURE_COORD_ARRAY_POINTER + 4 :  glClientActiveTexture(GL_TEXTURE0 + 4);
                                                            glTexCoordPointer(size, type, stride, array[a]); break;
               case GL_TEXTURE_COORD_ARRAY_POINTER + 5 :  glClientActiveTexture(GL_TEXTURE0 + 5);
                                                            glTexCoordPointer(size, type, stride, array[a]); break;
               case GL_TEXTURE_COORD_ARRAY_POINTER + 6 :  glClientActiveTexture(GL_TEXTURE0 + 6);
                                                            glTexCoordPointer(size, type, stride, array[a]); break;
               case GL_TEXTURE_COORD_ARRAY_POINTER + 7 :  glClientActiveTexture(GL_TEXTURE0 + 7);
                                                            glTexCoordPointer(size, type, stride, array[a]); break;
               case GL_NORMAL_ARRAY_POINTER :             glNormalPointer(type, stride, array[a]); break;
               case GL_POINT_SIZE_ARRAY_POINTER_OES :     glPointSizePointerOES(type, stride, array[a]); break;
               }

               a++;
            }
         }
         while (indx != -1);

         glClientActiveTexture(curTU);

         if (indicesBound)
            glDrawElements(GetU32(GLenum, 0), GetI32(GLsizei, 1), GetU32(GLenum, 2), GetPtr(GLvoid*, 3));
         else
            glDrawElements(GetU32(GLenum, 0), GetI32(GLsizei, 1), GetU32(GLenum, 2), (GLvoid*)array[0]);
      }
#else
      printf("EMULATOR DOES NOT SUPPORT ES1 CODE\n");
#endif
   }
   else if (esVer >= 2)
   {
      GLint indx = -1;
      GLint i = 7;
      GLint a = 1;

      do
      {
         indx = GetI32(GLint, i);
         i++;
         if (indx != -1)
         {
            GLint size = GetI32(GLint, i); i++;
            GLint type = GetI32(GLint, i); i++;
            GLint norm = GetI32(GLint, i); i++;
            GLint stride = GetI32(GLint, i); i++;
            GetArray(i, array[a]); i++;
            glVertexAttribPointer(m_replay->MapLocation(indx, GetCurProgram()), size, type, norm, stride, array[a]);
            a++;
         }
      }
      while (indx != -1);

      ProcessDeferredVAPs();

      if (indicesBound)
         glDrawElements(GetU32(GLenum, 0), GetI32(GLsizei, 1), GetU32(GLenum, 2), GetPtr(GLvoid*, 3));
      else
         glDrawElements(GetU32(GLenum, 0), GetI32(GLsizei, 1), GetU32(GLenum, 2), (GLvoid*)array[0]);
   }
}

// Locations
SPECIAL(glEnableVertexAttribArray) { glEnableVertexAttribArray(GetCurLocation(0)); }
SPECIAL(glGetActiveAttrib) { glGetActiveAttrib(GetProgram(GLuint, 0), GetLocation(1, 0), GetI32(GLsizei, 2), dummyGLint, dummyGLint, dummyGLenum, dummyGLchar); }


SPECIAL(glNormalPointer)
{
   bool b = GetB(bool, 3); // True if this is for a VBO - i.e. last arg is actually an index into the VBO
   if (b)
      glNormalPointer(GetI32(GLint, 0), GetI32(GLsizei, 1), GetPtr(GLvoid *, 2));
   else
   {
      // We do nothing for client side data - we'll handle this in glDraw*
   }
}

SPECIAL(glReadPixels)
{
   uint8_t *ptr = new uint8_t[(GetI32(GLsizei, 2) + 256) * GetI32(GLsizei, 3) * 4];
   glReadPixels(GetI32(GLint, 0), GetI32(GLint, 1), GetI32(GLsizei, 2), GetI32(GLsizei, 3), GetU32(GLenum, 4), GetU32(GLenum, 5), ptr);
   delete[] ptr;
}

SPECIAL(glScissor)
{
   uint32_t x = GetI32(GLint, 0);
   uint32_t y = GetI32(GLint, 1);
   uint32_t w = GetI32(GLint, 2);
   uint32_t h = GetI32(GLint, 3);

   GLint    p;
   glGetIntegerv(GL_FRAMEBUFFER_BINDING, &p);
   if (p == 0)
      m_replay->RescaleViewport(m_curDrawSurface, &x, &y, &w, &h);

   glScissor(x, y, w, h);
}

SPECIAL(glShaderBinary)
{
   GLsizei count = GetI32(GLsizei, 0);
   for (GLsizei i = 0; i < count; i++)
      dummyGLuint[i] = GetU32(GLuint, 5 + i);
   GetArray(5 + count, array[0]);
   glShaderBinary(count, dummyGLuint, GetU32(GLenum, 2), array[0], GetI32(GLsizei, 4));
}

SPECIAL(glShaderSource)
{
   uint32_t numStrings = GetU32(uint32_t, 1);
   if (numStrings > 0)
   {
      void **strArray = new void*[numStrings];
      uint32_t *strSizeArray = new uint32_t[numStrings];
      for (uint32_t i = 0; i < GetU32(uint32_t, 1); i++)
         strSizeArray[i] = GetArray(i + 4, strArray[i]);
      if (!m_replay->ReplaceShaderSource(GetShader(GLuint, 0)))
         glShaderSource(GetShader(GLuint, 0), GetI32(GLsizei, 1), (const GLchar**)strArray, (const GLint*)strSizeArray);
      delete [] strArray;
      delete [] strSizeArray;
   }
}

SPECIAL(glTexCoordPointer)
{
   bool b = GetB(bool, 4); // True if this is for a VBO - i.e. last arg is actually an index into the VBO
   if (b)
      glTexCoordPointer(GetI32(GLint, 0), GetU32(GLenum, 1), GetI32(GLsizei, 2), GetPtr(GLvoid *, 3));
   else
   {
      // We do nothing for client side data - we'll handle this in glDraw*
   }
}

SPECIAL(glTexImage2D)
{
   dummyGLint[0] = GetArray(9, array[0]);
   if (dummyGLint[0] == 0 && array[0] == NULL && m_packet.NumItems() > 11)
      array[0] = GetPtr(GLvoid *, 10);
   glTexImage2D(GetU32(GLenum, 0), GetI32(GLint, 1), GetI32(GLint, 2), GetI32(GLsizei, 3), GetI32(GLsizei, 4), GetI32(GLint, 5), GetU32(GLenum, 6), GetU32(GLenum, 7), array[0]);
}

SPECIAL(glTexSubImage2D)
{
   dummyGLint[0] = GetArray(9, array[0]);
   if (dummyGLint[0] == 0 && array[0] == NULL && m_packet.NumItems() > 11)
      array[0] = GetPtr(GLvoid *, 10);
   glTexSubImage2D(GetU32(GLenum, 0), GetI32(GLint, 1), GetI32(GLint, 2), GetI32(GLint, 3), GetI32(GLsizei, 4), GetI32(GLsizei, 5), GetU32(GLenum, 6), GetU32(GLenum, 7), array[0]);
}

SPECIAL(glUseProgram) { glUseProgram(GetProgram(GLuint, 0)); m_perContextState[m_curContext].m_curProgram = GetProgram(GLuint, 0); }

SPECIAL(glVertexAttrib1f) { glVertexAttrib1f(GetCurLocation(0), GetF(GLfloat, 1)); }

SPECIAL(glVertexAttrib1fv)
{
   GetArray(2, array[0]);
   glVertexAttrib1fv(GetCurLocation(0), (GLfloat*)array[0]);
}

SPECIAL(glVertexAttrib2f) { glVertexAttrib2f(GetCurLocation(0), GetF(GLfloat, 1), GetF(GLfloat, 2)); }

SPECIAL(glVertexAttrib2fv)
{
   GetArray(2, array[0]);
   glVertexAttrib2fv(GetCurLocation(0), (GLfloat*)array[0]);
}

SPECIAL(glVertexAttrib3f) { glVertexAttrib3f(GetCurLocation(0), GetF(GLfloat, 1), GetF(GLfloat, 2), GetF(GLfloat, 3)); }

SPECIAL(glVertexAttrib3fv)
{
   GetArray(2, array[0]);
   glVertexAttrib3fv(GetCurLocation(0), (GLfloat*)array[0]);
}

SPECIAL(glVertexAttrib4f) { glVertexAttrib4f(GetCurLocation(0), GetF(GLfloat, 1), GetF(GLfloat, 2), GetF(GLfloat, 3), GetF(GLfloat, 4)); }

SPECIAL(glVertexAttrib4fv)
{
   GetArray(2, array[0]);
   glVertexAttrib4fv(GetCurLocation(0), (GLfloat*)array[0]);
}

SPECIAL(glVertexAttribPointer)
{
   bool b = GetB(bool, 6); // True if this is for a VBO - i.e. last arg is actually an index into the VBO
   if (b)
   {
      GLint boundBuffer;
      glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &boundBuffer);

      GLint prog;
      glGetIntegerv(GL_CURRENT_PROGRAM, &prog);

      // We need to issue this here and also defer until just before the draw call
      glVertexAttribPointer(GetU32(GLuint, 0), GetI32(GLint, 1), GetU32(GLenum, 2), GetB(GLboolean, 3),
                            GetI32(GLsizei, 4), GetPtr(GLvoid *, 5));
      m_perContextState[m_curContext].m_deferredVAPs.push_back(DeferredVertexAttribPointer(GetU32(GLuint, 0), GetI32(GLint, 1), GetU32(GLenum, 2), GetB(GLboolean, 3),
                                                                  GetI32(GLsizei, 4), GetPtr(GLvoid *, 5), (GLuint)boundBuffer, (GLuint)prog));
   }
   else
   {
      // We do nothing for client side data - we'll handle this in glDraw*
   }
}

#if GL_ES_VERSION_3_0
SPECIAL(glVertexAttribIPointer)
{
   bool b = GetB(bool, 5); // True if this is for a VBO - i.e. last arg is actually an index into the VBO
   if (b)
   {
      GLint boundBuffer;
      glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &boundBuffer);

      GLint prog;
      glGetIntegerv(GL_CURRENT_PROGRAM, &prog);

      // We need to issue this here and also defer until just before the draw call
      glVertexAttribIPointer(GetU32(GLuint, 0), GetI32(GLint, 1), GetU32(GLenum, 2), GetI32(GLsizei, 3), GetPtr(GLvoid *, 4));
      m_perContextState[m_curContext].m_deferredVAPs.push_back(DeferredVertexAttribPointer(GetU32(GLuint, 0), GetI32(GLint, 1), GetU32(GLenum, 2),
                                                         GetI32(GLsizei, 3), GetPtr(GLvoid *, 4), (GLuint)boundBuffer, (GLuint)prog));
   }
   else
   {
      // We do nothing for client side data - we'll handle this in glDraw*
   }
}
#endif // GL_ES_VERSION_3_0

SPECIAL(glVertexPointer)
{
   bool b = GetB(bool, 4); // True if this is for a VBO - i.e. last arg is actually an index into the VBO
   if (b)
      glVertexPointer(GetI32(GLint, 0), GetU32(GLenum, 1), GetI32(GLsizei, 2), GetPtr(GLvoid *, 3));
   else
   {
      // We do nothing for client side data - we'll handle this in glDraw*
   }
}

SPECIAL(glViewport)
{
   uint32_t x = GetI32(GLint, 0);
   uint32_t y = GetI32(GLint, 1);
   uint32_t w = GetI32(GLint, 2);
   uint32_t h = GetI32(GLint, 3);

   GLint    p;
   glGetIntegerv(GL_FRAMEBUFFER_BINDING, &p);
   if (p == 0)
      m_replay->RescaleViewport(m_curDrawSurface, &x, &y, &w, &h);

   glViewport(x, y, w, h);
}

SPECIAL(glDrawTexsOES)
{
   if (m_replay->IncrSkipDrawCall())
      return;
   glDrawTexsOES(GetU32(GLshort, 0), GetI16(GLshort, 1), GetI16(GLshort, 2), GetI16(GLshort, 3), GetI16(GLshort, 4));
}

SPECIAL(glDrawTexiOES)
{
   if (m_replay->IncrSkipDrawCall())
      return;
   glDrawTexiOES(GetI32(GLint, 0), GetI32(GLint, 1), GetI32(GLint, 2), GetI32(GLint, 3), GetI32(GLint, 4));
}

SPECIAL(glDrawTexxOES)
{
   if (m_replay->IncrSkipDrawCall())
      return;
   glDrawTexxOES(GetU32(GLfixed, 0), GetU32(GLfixed, 1), GetU32(GLfixed, 2), GetU32(GLfixed, 3), GetU32(GLfixed, 4));
}

SPECIAL(glDrawTexsvOES)
{
   if (m_replay->IncrSkipDrawCall())
      return;
   GetArray(1, array[0]);
   glDrawTexsvOES((GLshort*)array[0]);
}

SPECIAL(glDrawTexivOES)
{
   if (m_replay->IncrSkipDrawCall())
      return;
   GetArray(1, array[0]);
   glDrawTexivOES((GLint*)array[0]);
}

SPECIAL(glDrawTexxvOES)
{
   if (m_replay->IncrSkipDrawCall())
      return;
   GetArray(1, array[0]);
   glDrawTexxvOES((GLfixed*)array[0]);
}

SPECIAL(glDrawTexfOES)
{
   if (m_replay->IncrSkipDrawCall())
      return;
   glDrawTexfOES(GetF(GLfloat, 0), GetF(GLfloat, 1), GetF(GLfloat, 2), GetF(GLfloat, 3), GetF(GLfloat, 4));
}

SPECIAL(glDrawTexfvOES)
{
   if (m_replay->IncrSkipDrawCall())
      return;
   GetArray(1, array[0]);
   glDrawTexfvOES((GLfloat*)array[0]);
}

SPECIAL(glPointSizePointerOES)
{
   bool b = GetB(bool, 3); // True if this is for a VBO - i.e. last arg is actually an index into the VBO
   if (b)
      glPointSizePointerOES(GetU32(GLenum, 0), GetI32(GLsizei, 1), GetPtr(GLvoid *, 2));
   else
   {
      // We do nothing for client side data - we'll handle this in glDraw*
   }
}

SPECIAL(eglGetDisplay)
{
   EGLNativeDisplayType nativeDisplay = GetPtr(EGLNativeDisplayType, 0);
   if (nativeDisplay != EGL_DEFAULT_DISPLAY)
   {
      printf("*************************  WARNING  *********************************\n");
      printf(" User defined display, retargeting at EGL_DEFAULT_DISPLAY\n");
      printf("*********************************************************************\n");
   }
   m_replay->AddDisplayMapping(m_retPacket.Item(1).GetUInt32(), eglGetDisplay(EGL_DEFAULT_DISPLAY));
}

SPECIAL(eglInitialize)
{
   if (!m_replay->HasDisplayMapping(m_packet.Item(1).GetUInt32()))
      m_replay->AddDisplayMapping(m_packet.Item(1).GetUInt32(), eglGetDisplay(EGL_DEFAULT_DISPLAY));

   EGLDisplay display = GetEGLDisplay(EGLDisplay, 0);

   if (eglInitialize(display, &dummyGLint[0], &dummyGLint[1]))
      m_replay->DisplayInited(display);
}

SPECIAL(eglTerminate)
{
   EGLDisplay display = GetEGLDisplay(EGLDisplay, 0);
   eglTerminate(display);
   m_replay->DisplayTermed(display);

}

// We only need this to prevent warnings from the base class impl
SPECIAL(eglChooseConfig) { eglChooseConfig(GetEGLDisplay(EGLDisplay, 0), GetArrayPtr(const EGLint *, 5), dummyEGLConfig, GetI32(EGLint, 3), dummyEGLint); }

// We only need this to prevent warnings from the base class impl
SPECIAL(eglGetConfigs) { eglGetConfigs(GetEGLDisplay(EGLDisplay, 0), dummyEGLConfig, GetI32(EGLint, 2), dummyEGLint); }

SPECIAL(eglGetConfigAttrib)
{
   // The original config ID will never be a match, so leave this to MatchConfig
   //eglGetConfigAttrib(GetEGLDisplay(EGLDisplay, 0), GetEGLConfig(EGLConfig, 1), GetI32(EGLint, 2), dummyGLint);
}

SPECIAL(eglCreateWindowSurface)
{
   GetArray(4, array[0]);

   if (m_retPacket.NumItems() > 9)
   {
      dummyEGLConfig[0] = MatchConfig(m_retPacket.Item(4).GetUInt32(), m_retPacket.Item(5).GetUInt32(),
                                m_retPacket.Item(6).GetUInt32(), m_retPacket.Item(7).GetUInt32(),
                                m_retPacket.Item(8).GetUInt32(), m_retPacket.Item(9).GetUInt32(),
                                m_retPacket.Item(10).GetUInt32());
   }
   else
   {
      // Older captures don't have the depth, stencil & samples data in the packet
      dummyEGLConfig[0] = MatchConfig(m_retPacket.Item(4).GetUInt32(), m_retPacket.Item(5).GetUInt32(),
                                m_retPacket.Item(6).GetUInt32(), m_retPacket.Item(7).GetUInt32());
   }
   if (!m_replay->HasConfigMapping(GetU32(uint32_t, 1)))
   {
      EGLint ver[3] = { EGL_CONTEXT_CLIENT_VERSION, (EGLint)m_delayedContextESVer, EGL_NONE };

      m_replay->AddConfigMapping(GetU32(uint32_t, 1), dummyEGLConfig[0]);

      if (m_retPacket.NumItems() <= 9)
      {
         // Support for older captures - new ones will have created the context during the real eglCreateContext
         if (m_delayedContextESVer != 0)
         {
            m_replay->AddContextMapping((uintptr_t)m_delayedContextContext, eglCreateContext(m_delayedContextDisplay,
               GetEGLConfig(EGLConfig, 1), m_delayedContextShare, ver));
         }
         else
         {
            m_replay->AddContextMapping((uintptr_t)m_delayedContextContext, eglCreateContext(m_delayedContextDisplay,
               GetEGLConfig(EGLConfig, 1), m_delayedContextShare, NULL));
         }
      }
   }

   {
      EGLSurface surf;

      if (m_replay->DisplayCurSurface())
      {
         surf = eglCreateWindowSurface(GetEGLDisplay(EGLDisplay, 0), GetEGLConfig(EGLConfig, 1),
                                       (EGLNativeWindowType)m_replay->GetNativeWindow(GetU32(uint32_t, 2),
                                       m_retPacket.Item(2).GetUInt32(), m_retPacket.Item(3).GetUInt32(),
                                       m_retPacket.Item(4).GetUInt32(), m_retPacket.Item(5).GetUInt32(),
                                       m_retPacket.Item(6).GetUInt32(), m_retPacket.Item(7).GetUInt32()),
                                       (const EGLint *)array[0]);
      }
      else
      {
         printf("*************************  WARNING  *********************************\n");
         printf("Multiple surface creates found - not displaying surface index %d\n",
                m_replay->GetCurSurfaceIndex());
         printf("Use the 'displaySurface' option to control which surface is displayed\n");
         printf("*********************************************************************\n");

         EGLint attribs[] = { EGL_WIDTH, 0, EGL_HEIGHT, 0, EGL_NONE };

         attribs[1] = m_retPacket.Item(2).GetUInt32();
         attribs[3] = m_retPacket.Item(3).GetUInt32();

         surf = eglCreatePbufferSurface(GetEGLDisplay(EGLDisplay, 0), GetEGLConfig(EGLConfig, 1), attribs);
      }

      m_replay->IncrementCurSurfaceIndex();

      if (surf != NULL)
      {
         m_replay->AddSurfaceMapping(m_retPacket.Item(1).GetUInt32(), surf);
         m_replay->AddWindowSizeMapping(surf, WindowSize(m_retPacket.Item(2).GetUInt32(), m_retPacket.Item(3).GetUInt32()));
      }
   }
}

SPECIAL(eglCreatePbufferSurface)
{
   GetArray(3, array[0]);
   if (m_retPacket.NumItems() > 9)
   {
      dummyEGLConfig[0] = MatchConfig(m_retPacket.Item(4).GetUInt32(), m_retPacket.Item(5).GetUInt32(),
                                m_retPacket.Item(6).GetUInt32(), m_retPacket.Item(7).GetUInt32(),
                                m_retPacket.Item(8).GetUInt32(), m_retPacket.Item(9).GetUInt32(),
                                m_retPacket.Item(10).GetUInt32());
   }
   else
   {
      dummyEGLConfig[0] = MatchConfig(m_retPacket.Item(4).GetUInt32(), m_retPacket.Item(5).GetUInt32(),
                                m_retPacket.Item(6).GetUInt32(), m_retPacket.Item(7).GetUInt32());
   }
   if (!m_replay->HasConfigMapping(GetU32(uint32_t, 1)))
   {
      EGLint ver[3] = { EGL_CONTEXT_CLIENT_VERSION, (EGLint)m_delayedContextESVer, EGL_NONE };

      m_replay->AddConfigMapping(GetU32(uint32_t, 1), dummyEGLConfig[0]);

      if (m_retPacket.NumItems() <= 9)
      {
         // Support for older captures - new ones will have created the context during the real eglCreateContext
         if (m_delayedContextESVer != 0)
         {
            m_replay->AddContextMapping((uintptr_t)m_delayedContextContext, eglCreateContext(m_delayedContextDisplay,
               GetEGLConfig(EGLConfig, 1), m_delayedContextShare, ver));
         }
         else
         {
            m_replay->AddContextMapping((uintptr_t)m_delayedContextContext, eglCreateContext((EGLDisplay)m_delayedContextDisplay,
               GetEGLConfig(EGLConfig, 1), m_delayedContextShare, NULL));
         }
      }
   }
   dummyEGLSurface[0] = eglCreatePbufferSurface(GetEGLDisplay(EGLDisplay, 0), GetEGLConfig(EGLConfig, 1), (const EGLint *)array[0]);
   m_replay->AddSurfaceMapping(m_retPacket.Item(1).GetUInt32(), dummyEGLSurface[0]);
}

SPECIAL(eglCreatePixmapSurface)
{
   GetArray(4, array[0]);
   if (m_retPacket.NumItems() > 8)
   {
      dummyEGLConfig[0] = MatchConfig(m_retPacket.Item(4).GetUInt32(), m_retPacket.Item(5).GetUInt32(),
                                m_retPacket.Item(6).GetUInt32(), m_retPacket.Item(7).GetUInt32(),
                                m_retPacket.Item(8).GetUInt32(), m_retPacket.Item(9).GetUInt32(),
                                m_retPacket.Item(10).GetUInt32());
   }
   else
   {
      dummyEGLConfig[0] = MatchConfig(m_retPacket.Item(4).GetUInt32(), m_retPacket.Item(5).GetUInt32(),
                                m_retPacket.Item(6).GetUInt32(), m_retPacket.Item(7).GetUInt32());
   }
   m_replay->AddConfigMapping(GetU32(uint32_t, 1), dummyEGLConfig[0]);
   dummyEGLSurface[0] = eglCreatePixmapSurface(GetEGLDisplay(EGLDisplay, 0), GetEGLConfig(EGLConfig, 1),
                                         (EGLNativePixmapType)m_replay->GetNativePixmap(GetU32(uint32_t, 2),
                                         m_retPacket.Item(2).GetUInt32(), m_retPacket.Item(3).GetUInt32(),
                                         m_retPacket.Item(4).GetUInt32(), m_retPacket.Item(5).GetUInt32(),
                                         m_retPacket.Item(6).GetUInt32(), m_retPacket.Item(7).GetUInt32()),
                                         (const EGLint *)array[0]);
   m_replay->AddSurfaceMapping(m_retPacket.Item(1).GetUInt32(), dummyEGLSurface[0]);
}

SPECIAL(eglCreateContext)
{
   GetArray(5, array[0]);
   if (m_replay->HasConfigMapping(m_packet.Item(2).GetUInt32()))
   {
      m_replay->AddContextMapping(m_retPacket.Item(1).GetUInt32(),
                                  eglCreateContext(GetEGLDisplay(EGLDisplay, 0), GetEGLConfig(EGLConfig, 1), GetEGLContext(EGLContext, 2), (const EGLint *)array[0]));
   }
   else if (m_packet.NumItems() > 7)
   {
      // Newer captures have config matching data in the packet so we can create the context now without delay
      EGLConfig cfg = (EGLConfig)MatchConfig(m_packet.Item(9).GetUInt32(), m_packet.Item(10).GetUInt32(), m_packet.Item(11).GetUInt32(), m_packet.Item(12).GetUInt32(),
                                             m_packet.Item(7).GetUInt32(), m_packet.Item(8).GetUInt32(), m_packet.Item(13).GetUInt32());
      m_replay->AddConfigMapping(GetU32(uint32_t, 1), cfg);
      m_replay->AddContextMapping(m_retPacket.Item(1).GetUInt32(),
                                  eglCreateContext(GetEGLDisplay(EGLDisplay, 0), GetEGLConfig(EGLConfig, 1), GetEGLContext(EGLContext, 2), (const EGLint *)array[0]));
   }
   else
   {
      // Legacy versions don't have config matching data in the packet
      m_delayedContextContext = (EGLContext)(uintptr_t)m_retPacket.Item(1).GetUInt32();
      m_delayedContextDisplay = GetEGLDisplay(EGLDisplay, 0);
      m_delayedContextConfig = (EGLConfig)(uintptr_t)m_packet.Item(2).GetUInt32();
      m_delayedContextShare = GetEGLContext(EGLContext, 2);
      m_delayedContextESVer = 1;
      if (array[0])
      {
         const EGLint *attribList = (const EGLint *)array[0];
         EGLint attribute;
         while ((attribute = *attribList++) != EGL_NONE)
         {
            if (attribute == EGL_CONTEXT_CLIENT_VERSION)
               m_delayedContextESVer = *attribList++;
         }
      }
   }
}

SPECIAL(eglMakeCurrent)
{
   // If we have more than 7 packet items then we will have extra info required to match the config and create the context.
   // Older captures don't have the extra data
   if (m_packet.NumItems() > 7 &&
       (EGLContext)(uintptr_t)m_packet.Item(4).GetUInt32() != EGL_NO_CONTEXT &&
       !m_replay->HasConfigMapping(m_packet.Item(12).GetUInt32()))
   {
      EGLConfig cfg = (EGLConfig)MatchConfig(m_packet.Item(7).GetUInt32(), m_packet.Item(8).GetUInt32(), m_packet.Item(9).GetUInt32(), m_packet.Item(10).GetUInt32(),
                                             m_packet.Item(5).GetUInt32(), m_packet.Item(6).GetUInt32(), m_packet.Item(11).GetUInt32());
      m_replay->AddConfigMapping(m_packet.Item(12).GetUInt32(), cfg);
      m_replay->AddContextMapping(m_retPacket.Item(4).GetUInt32(),
                                  eglCreateContext(GetEGLDisplay(EGLDisplay, 0), cfg, GetEGLContext(EGLContext, 2), (const EGLint *)array[0]));
   }
   eglMakeCurrent(GetEGLDisplay(EGLDisplay, 0), GetEGLSurface(EGLSurface, 1), GetEGLSurface(EGLSurface, 2), GetEGLContext(EGLContext, 3));

   // Examine the extensions
   GetGLExtensions();
}

SPECIAL(eglGetCurrentContext) { m_replay->AddContextMapping(m_retPacket.Item(1).GetUInt32(), eglGetCurrentContext()); }
SPECIAL(eglGetCurrentDisplay) { m_replay->AddDisplayMapping(m_retPacket.Item(1).GetUInt32(), eglGetCurrentDisplay()); }
SPECIAL(eglSwapBuffers)
{
   if (m_replay->SkipFrame(m_curFrame++))
      return;
   // callback to replay to possibly intercept
   m_replay->FrameDone(m_curFrame - 1);
   eglSwapBuffers(GetEGLDisplay(EGLDisplay, 0), GetEGLSurface(EGLSurface, 1));
}

SPECIAL(eglCreateImageKHR)
{
   GetArray(5, array[0]);
   EGLenum target = GetU32(GLenum, 2);
   EGLClientBuffer buffer;
   switch (target)
   {
   case EGL_GL_TEXTURE_2D_KHR:
   case EGL_GL_TEXTURE_3D_KHR:
   case EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_X_KHR:
   case EGL_GL_TEXTURE_CUBE_MAP_NEGATIVE_X_KHR:
   case EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_Y_KHR:
   case EGL_GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_KHR:
   case EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_Z_KHR:
   case EGL_GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_KHR:
   case EGL_GL_RENDERBUFFER_KHR:
      buffer = GetPtr(EGLClientBuffer, 3);
      break;
   case EGL_NATIVE_PIXMAP_KHR:
      buffer =  (new bsg::NativePixmap(16, 16, bsg::NativePixmap::eABGR8888_TEXTURE))->EGLPixmap();
      break;
   default:
      target = 0;
      break;
   }

   if (target) {
      EGLDisplay display = GetEGLDisplay(EGLDisplay, 0);
      EGLContext context = GetEGLContext(EGLContext, 1);
      const EGLint *attrib_list = (const EGLint *)array[0];
      EGLImageKHR image = eglCreateImageKHR(display, context, target, buffer, attrib_list);
      m_replay->AddEGLImageMapping(m_retPacket.Item(1).GetUInt32(), image);
   }
}

SPECIAL(eglCreateImage)
{
   run_eglCreateImageKHR();
}

SPECIAL(eglCreateSyncKHR)
{
#if EGL_KHR_fence_sync
   if (m_packet.NumItems() > 5) WarnDataNotHandled("eglCreateSyncKHR");
   PFNEGLCREATESYNCKHRPROC f = (PFNEGLCREATESYNCKHRPROC)eglGetProcAddress("eglCreateSyncKHR");
   if (f != NULL)
   {
      EGLSyncKHR sync = f(GetEGLDisplay(EGLDisplay, 0), GetU32(EGLenum, 1), GetArrayPtr(const EGLint *, 3));
      m_replay->AddEGLSyncMapping(m_retPacket.Item(1).GetUInt32(), sync);
   }
#else
   WarnNotAvailable("eglCreateSyncKHR");
#endif
}

SPECIAL(glRenderbufferStorage)
{
   GLenum comp = GetU32(GLenum, 1);

   if (comp == GL_DEPTH_COMPONENT32_OES && m_extensions.find("GL_OES_depth32") == std::string::npos)
      comp = GL_DEPTH_COMPONENT24_OES; // We can't support 32, use 24 instead

   glRenderbufferStorage(GetU32(GLenum, 0), comp, GetI32(GLsizei, 2), GetI32(GLsizei, 3));
}

SPECIAL(glLinkProgram)
{
   GLuint program = GetProgram(GLuint, 0);
   glLinkProgram(program);

#if GL_ES_VERSION_3_0
   // The latest captures contain mapping data for uniform block indices.
   // Use the information to create mapping tables for the index data.
   if (m_retPacket.NumItems() > 5)
   {
      uint32_t version = m_retPacket.Item(4).GetUInt32();
      if (version != 1)
         return;

      uint32_t numBlocks = m_retPacket.Item(5).GetUInt32();
      //uint32_t maxNameLen  = m_retPacket.Item(6).GetUInt32(); not used

      uint32_t i = 7;

      for (uint32_t b = 0; b < numBlocks; b++)
      {
         std::string name(m_retPacket.Item(i++).GetCharPtr());
         GLuint bi = glGetUniformBlockIndex(program, name.c_str());
         m_replay->AddUniformBlockIndexMapping(b, bi, program);
      }
   }
#endif
}

#if GL_ES_VERSION_3_0
SPECIAL(glBlitFramebuffer)
{
   if (m_replay->SkipFrame(m_curFrame))
      return;

   glBlitFramebuffer(GetI32(GLint, 0), GetI32(GLint, 1), GetI32(GLint, 2), GetI32(GLint, 3), GetI32(GLint, 4), GetI32(GLint, 5), GetI32(GLint, 6), GetI32(GLint, 7), GetU32(GLbitfield, 8), GetU32(GLenum, 9));
}

SPECIAL(glDrawRangeElements)
{
   if (m_replay->SkipFrame(m_curFrame))
      return;

   if (m_replay->IncrSkipDrawCall())
      return;

   bool indicesBound = GetB(bool, 6);

   if (!indicesBound)
      GetArray(7, array[0]);

   // Get ES1 or 2 or 3
   uint32_t esVer = GetU32(uint32_t, 8);
   if (esVer >= 2)
   {
      GLint indx = -1;
      GLint i = 9;
      GLint a = 1;

      do
      {
         indx = GetI32(GLint, i);
         i++;
         if (indx != -1)
         {
            GLint size = GetI32(GLint, i); i++;
            GLint type = GetI32(GLint, i); i++;
            GLint norm = GetI32(GLint, i); i++;
            GLint stride = GetI32(GLint, i); i++;
            GetArray(i, array[a]); i++;
            glVertexAttribPointer(m_replay->MapLocation(indx, GetCurProgram()), size, type, norm, stride, array[a]);
            a++;
         }
      }
      while (indx != -1);

      ProcessDeferredVAPs();

      if (indicesBound)
         glDrawRangeElements(GetU32(GLenum, 0), GetU32(GLuint, 1), GetU32(GLuint, 2), GetI32(GLsizei, 3), GetU32(GLenum, 4), GetPtr(GLvoid*, 5));
      else
         glDrawRangeElements(GetU32(GLenum, 0), GetU32(GLuint, 1), GetU32(GLuint, 2), GetI32(GLsizei, 3), GetU32(GLenum, 4), (GLvoid*)array[0]);
   }
}

SPECIAL(glTexImage3D)
{
   dummyGLint[0] = GetArray(10, array[0]);
   if (dummyGLint[0] == 0 && array[0] == NULL && m_packet.NumItems() > 12)
      array[0] = GetPtr(GLvoid *, 11);
   glTexImage3D(GetU32(GLenum, 0), GetI32(GLint, 1), GetI32(GLint, 2), GetI32(GLsizei, 3), GetI32(GLsizei, 4), GetI32(GLsizei, 5), GetI32(GLint, 6), GetU32(GLenum, 7), GetU32(GLenum, 8), array[0]);
}

SPECIAL(glTexSubImage3D)
{
   dummyGLint[0] = GetArray(11, array[0]);
   if (dummyGLint[0] == 0 && array[0] == NULL && m_packet.NumItems() > 13)
      array[0] = GetPtr(GLvoid *, 12);
   glTexSubImage3D(GetU32(GLenum, 0), GetI32(GLint, 1), GetI32(GLint, 2), GetI32(GLint, 3), GetI32(GLint, 4), GetI32(GLsizei, 5), GetI32(GLsizei, 6), GetI32(GLsizei, 7), GetU32(GLenum, 8), GetU32(GLenum, 9), array[0]);
}

SPECIAL(glCompressedTexImage3D)
{
   dummyGLint[0] = GetArray(9, array[0]);
   if (dummyGLint[0] == 0 && array[0] == NULL && m_packet.NumItems() > 11)
      array[0] = GetPtr(GLvoid *, 10);
   glCompressedTexImage3D(GetU32(GLenum, 0), GetI32(GLint, 1), GetI32(GLenum, 2), GetI32(GLsizei, 3), GetI32(GLsizei, 4), GetI32(GLsizei, 5), GetI32(GLint, 6), GetI32(GLsizei, 7), array[0]);
}

SPECIAL(glCompressedTexSubImage3D)
{
   dummyGLint[0] = GetArray(11, array[0]);
   if (dummyGLint[0] == 0 && array[0] == NULL && m_packet.NumItems() > 13)
      array[0] = GetPtr(GLvoid *, 12);
   glCompressedTexSubImage3D(GetU32(GLenum, 0), GetI32(GLint, 1), GetI32(GLint, 2), GetI32(GLint, 3), GetI32(GLint, 4), GetI32(GLsizei, 5), GetI32(GLsizei, 6), GetI32(GLsizei, 7), GetU32(GLenum, 8), GetI32(GLsizei, 9), array[0]);
}

SPECIAL(glGenQueries)
{
   GLsizei numQs = GetI32(GLsizei, 0);
   glGenQueries(numQs, (GLuint*)dummyGLint);

   for (GLsizei si = 0; si < numQs; si++)
   {
      if (m_retPacket.NumItems() > 2)
         m_replay->AddQueryMapping(dummyGLint[si], m_retPacket.Item(si + 2).GetUInt32());
      else
         m_replay->AddQueryMapping(dummyGLint[si], dummyGLint[si]);
   }
}

SPECIAL(glDeleteQueries)
{
   GetArray(2, array[0]);
   GLsizei n = GetI32(GLsizei, 0);
   unsigned int * p = (GLuint*)array[0];
   // Map the queries
   for (GLsizei si = 0; si < n; si++)
      p[si] = m_replay->MapQuery(p[si]);

   glDeleteQueries(n, p);
}

SPECIAL(glUnmapBuffer)
{
   GLenum target = GetU32(GLenum, 0);
   bool   writing = GetU32(GLuint, 1) != 0;
   if (writing)
   {
      //GLintptr   offset = GetI32(GLintptr, 2);  unused
      GLsizeiptr len = GetI32(GLsizeiptr, 3);
      GLvoid     *ptr;
      GetArray(4, array[0]);

      glGetBufferPointerv(target, GL_BUFFER_MAP_POINTER, &ptr);

      if (ptr != NULL && array[0] != NULL)
         memcpy(ptr, array[0], len);
   }
   glUnmapBuffer(target);
}

SPECIAL(glDrawBuffers)
{
   GetArray(2, array[0]);
   GLsizei n = GetI32(GLsizei, 0);
   const GLenum * p = (GLenum*)array[0];
   glDrawBuffers(n, p);
}

SPECIAL(glFlushMappedBufferRange)
{
   GLenum      target = GetU32(GLenum, 0);
   GLintptr    offset = GetI32(GLintptr, 1);
   GLsizeiptr  len    = GetI32(GLsizeiptr, 2);

   bool writing = GetU32(GLuint, 3) != 0;
   if (writing)
   {
      //GLintptr mappedOffset = GetI32(GLintptr, 4);  unused
      GLsizeiptr sentLen = GetI32(GLsizeiptr, 5);
      GLvoid     *ptr;
      GetArray(6, array[0]);

      glGetBufferPointerv(target, GL_BUFFER_MAP_POINTER, &ptr);

      memcpy((void*)((uintptr_t)ptr + offset), array[0], sentLen);
   }
   glFlushMappedBufferRange(target, offset, len);
}

SPECIAL(glDeleteVertexArrays)
{
   GetArray(2, array[0]);
   GLsizei n = GetI32(GLsizei, 0);
   GLuint *p = (GLuint*)array[0];
   for (GLsizei si = 0; si < n; si++)
      p[si] = m_replay->MapVertexArray(p[si]);

   glDeleteVertexArrays(n, p);
}

SPECIAL(glGenVertexArrays)
{
   GLsizei numVAs = GetI32(GLsizei, 0);
   glGenVertexArrays(numVAs, (GLuint*)dummyGLint);

   for (GLsizei si = 0; si < numVAs; si++)
   {
      if (m_retPacket.NumItems() > 2)
         m_replay->AddVertexArrayMapping(dummyGLint[si], m_retPacket.Item(si + 2).GetUInt32());
      else
         m_replay->AddVertexArrayMapping(dummyGLint[si], dummyGLint[si]);
   }
}

SPECIAL(glTransformFeedbackVaryings)
{
   GLsizei count = GetI32(GLsizei, 1);
   for (GLsizei i = 0; i < count; i++)
      array[i] = (void*)GetCP(const GLchar*, 4 + i);

   glTransformFeedbackVaryings(GetU32(GLuint, 0), count, (const GLchar *const *)array, GetU32(GLenum, 3));
}

SPECIAL(glGetTransformFeedbackVarying)
{
   GLsizei bufSize = GetI32(GLsizei, 2);
   GLchar *name = new GLchar[bufSize + 1];
   GLuint lengthPtr = GetU32(GLuint, 3);

   glGetTransformFeedbackVarying(GetU32(GLuint, 0), GetU32(GLuint, 1), bufSize,
                                 lengthPtr == 0 ? NULL : &dummyGLint[0], &dummyGLint[1],
                                 &dummyGLenum[0], name);
   delete [] name;
}

SPECIAL(glGetVertexAttribIiv) { glGetVertexAttribIiv(GetCurLocation(0), GetU32(GLenum, 1), dummyGLint); }
SPECIAL(glGetVertexAttribIuiv) { glGetVertexAttribIuiv(GetCurLocation(0), GetU32(GLenum, 1), dummyGLuint); }
SPECIAL(glVertexAttribI4i) { glVertexAttribI4i(GetCurLocation(0), GetI32(GLint, 1), GetI32(GLint, 2), GetI32(GLint, 3), GetI32(GLint, 4)); }
SPECIAL(glVertexAttribI4ui) { glVertexAttribI4ui(GetCurLocation(0), GetU32(GLuint, 1), GetU32(GLuint, 2), GetU32(GLuint, 3), GetU32(GLuint, 4)); }
SPECIAL(glVertexAttribI4iv)
{
   GetArray(2, array[0]);
   glVertexAttribI4iv(GetCurLocation(0), (GLint*)array[0]);
}

SPECIAL(glVertexAttribI4uiv)
{
   GetArray(2, array[0]);
   glVertexAttribI4uiv(GetCurLocation(0), (GLuint*)array[0]);
}

SPECIAL(glGetUniformIndices)
{
   int32_t numUniforms = GetI32(GLsizei, 1);
   if (numUniforms > 0)
   {
      void **strArray = new void*[numUniforms];
      for (int32_t i = 0; i < numUniforms; i++)
         GetArray(i + 4, strArray[i]);
      glGetUniformIndices(GetProgram(GLuint, 0), GetI32(GLsizei, 1), (const GLchar* const*)strArray, dummyGLuint);
      delete [] strArray;
   }
}
SPECIAL(glGetActiveUniformsiv)
{
   GetArray(5, array[0]);
   glGetActiveUniformsiv(GetProgram(GLuint, 0), GetI32(GLsizei, 1), (const GLuint *)array[0], GetU32(GLenum, 3), dummyGLint);
}
SPECIAL(glDrawArraysInstanced)
{
   if (m_replay->SkipFrame(m_curFrame))
      return;
   if (m_replay->IncrSkipDrawCall())
      return;

   {
      // Get ES1 or 2 or 3
      uint32_t esVer = GetU32(uint32_t, 4);

      // We need to get any client side data from the packet now and install it
      if (esVer >= 2)
      {
         GLint indx = -1;
         GLint i = 5;
         GLint a = 0;

         do
         {
            indx = GetI32(GLint, i); i++;
            if (indx != -1)
            {
               GLint size = GetI32(GLint, i); i++;
               GLint type = GetI32(GLint, i); i++;
               GLint norm = GetI32(GLint, i); i++;
               GLint stride = GetI32(GLint, i); i++;
               GetArray(i, array[a]); i++;

               glVertexAttribPointer(m_replay->MapLocation(indx, GetCurProgram()), size, type, norm, stride, array[a]);
               a++;
            }
         }
         while (indx != -1);

         ProcessDeferredVAPs();
      }
   }
   glDrawArraysInstanced(GetU32(GLenum, 0), GetI32(GLint, 1), GetI32(GLsizei, 2), GetI32(GLsizei, 3));
}

SPECIAL(glDrawElementsInstanced)
{
   if (m_replay->SkipFrame(m_curFrame))
      return;
   if (m_replay->IncrSkipDrawCall())
      return;

   bool indicesBound = GetB(bool, 5);

   if (!indicesBound)
      GetArray(6, array[0]);

   // Get ES1 or 2 or 3
   uint32_t esVer = GetU32(uint32_t, 7);

   if (esVer >= 2)
   {
      GLint indx = -1;
      GLint i = 8;
      GLint a = 1;

      do
      {
         indx = GetI32(GLint, i);
         i++;
         if (indx != -1)
         {
            GLint size = GetI32(GLint, i); i++;
            GLint type = GetI32(GLint, i); i++;
            GLint norm = GetI32(GLint, i); i++;
            GLint stride = GetI32(GLint, i); i++;
            GetArray(i, array[a]); i++;
            glVertexAttribPointer(m_replay->MapLocation(indx, GetCurProgram()), size, type, norm, stride, array[a]);
            a++;
         }
      }
      while (indx != -1);

      ProcessDeferredVAPs();

      if (indicesBound)
         glDrawElementsInstanced(GetU32(GLenum, 0), GetI32(GLsizei, 1), GetU32(GLenum, 2), GetPtr(GLvoid*, 3), GetI32(GLsizei, 4));
      else
         glDrawElementsInstanced(GetU32(GLenum, 0), GetI32(GLsizei, 1), GetU32(GLenum, 2), (GLvoid*)array[0], GetI32(GLsizei, 4));
   }
}

SPECIAL(glFenceSync)
{
   GLsync s = glFenceSync(GetU32(GLenum, 0), GetU32(GLbitfield, 1));
   m_replay->AddSyncMapping(m_retPacket.Item(1).GetUInt32(), s);
}

SPECIAL(glClientWaitSync)
{
   GLuint64 timeout = GetU32(GLuint, 2) | (GetU32(GLuint64, 3) << 32);
   glClientWaitSync(GetSync(GLsync, 0), GetU32(GLbitfield, 1), timeout);
}

SPECIAL(glWaitSync)
{
   GLuint64 timeout = GetU32(GLuint, 2) | (GetU32(GLuint64, 3) << 32);
   glWaitSync(GetSync(GLsync, 0), GetU32(GLbitfield, 1), timeout);
}

SPECIAL(glGetSynciv)
{
   GLuint lengthPtr = GetU32(GLuint, 3);
   glGetSynciv(GetSync(GLsync, 0), GetU32(GLenum, 1), GetI32(GLsizei, 2), lengthPtr == 0 ? NULL : &dummyGLint[0], dummyGLint);
}

SPECIAL(glGenSamplers)
{
   GLsizei numSs = GetI32(GLsizei, 0);
   glGenSamplers(numSs, (GLuint*)dummyGLint);

   for (GLsizei si = 0; si < numSs; si++)
   {
      if (m_retPacket.NumItems() > 2)
         m_replay->AddSamplerMapping(dummyGLint[si], m_retPacket.Item(si + 2).GetUInt32());
      else
         m_replay->AddSamplerMapping(dummyGLint[si], dummyGLint[si]);
   }
}

SPECIAL(glDeleteSamplers)
{
   GetArray(2, array[0]);
   GLsizei n = GetI32(GLsizei, 0);
   GLuint *p = (GLuint*)array[0];
   for (GLsizei si = 0; si < n; si++)
      p[si] = m_replay->MapSampler(p[si]);

   glDeleteSamplers(n, p);
}

SPECIAL(glDeleteTransformFeedbacks)
{
   GetArray(2, array[0]);
   GLsizei n = GetI32(GLsizei, 0);
   GLuint *p = (GLuint*)array[0];
   for (GLsizei si = 0; si < n; si++)
      p[si] = m_replay->MapTF(p[si]);

   glDeleteTransformFeedbacks(n, p);
}

SPECIAL(glGenTransformFeedbacks)
{
   GLsizei numTFs = GetI32(GLsizei, 0);
   glGenTransformFeedbacks(numTFs, (GLuint*)dummyGLint);

   for (GLsizei si = 0; si < numTFs; si++)
   {
      if (m_retPacket.NumItems() > 2)
         m_replay->AddTFMapping(dummyGLint[si], m_retPacket.Item(si + 2).GetUInt32());
      else
         m_replay->AddTFMapping(dummyGLint[si], dummyGLint[si]);
   }
}

SPECIAL(glGetProgramBinary)
{
   GLsizei bufSize = GetI32(GLsizei, 1);
   uint8_t *binBuf = new uint8_t[bufSize];
   glGetProgramBinary(GetProgram(GLuint, 0), bufSize, (GLsizei*)dummyGLint, (GLenum*)dummyGLuint, (void*)binBuf);
   delete [] binBuf;
}

SPECIAL(glDrawArraysIndirect)
{
   if (m_replay->SkipFrame(m_curFrame))
      return;

   if (m_replay->IncrSkipDrawCall())
      return;

   if (m_packet.NumItems() > 4)
      WarnDataNotHandled("glDrawArraysIndirect");
   if (m_replay->GetDeviceCaps().m_has_GL_ES_VERSION_3_1 && m_replay->GetDispatch().real_glDrawArraysIndirect)
      m_replay->GetDispatch().real_glDrawArraysIndirect(GetU32(GLenum, 0), GetArrayPtr(const void *, 2));
   else
      WarnNotAvailable("glDrawArraysIndirect");
}

SPECIAL(glDrawElementsIndirect)
{
   if (m_replay->SkipFrame(m_curFrame))
      return;

   if (m_replay->IncrSkipDrawCall())
      return;

   if (m_packet.NumItems() > 5)
      WarnDataNotHandled("glDrawElementsIndirect");
   if (m_replay->GetDeviceCaps().m_has_GL_ES_VERSION_3_1 && m_replay->GetDispatch().real_glDrawElementsIndirect)
      m_replay->GetDispatch().real_glDrawElementsIndirect(GetU32(GLenum, 0), GetU32(GLenum, 1), GetArrayPtr(const void *, 3));
   else
      WarnNotAvailable("glDrawElementsIndirect");
}

SPECIAL(glCreateShaderProgramv)
{
   GLsizei numStrings = GetI32(GLsizei, 1);
   if (numStrings > 0)
   {
      void **strArray = new void*[numStrings];
      for (int i = 0; i < numStrings; i++)
         GetArray(i + 3, strArray[i]);

      if (m_packet.NumItems() > 5)
         WarnDataNotHandled("glCreateShaderProgramv");
      if (m_replay->GetDeviceCaps().m_has_GL_ES_VERSION_3_1 && m_replay->GetDispatch().real_glCreateShaderProgramv)
         m_replay->GetDispatch().real_glCreateShaderProgramv(GetU32(GLenum, 0), numStrings, (const GLchar *const*)strArray);
      else
         WarnNotAvailable("glCreateShaderProgramv");

      delete[] strArray;
   }
}

SPECIAL(glDeleteProgramPipelines)
{
   GetArray(2, array[0]);
   GLsizei n = GetI32(GLsizei, 0);
   unsigned int *p = (GLuint*)array[0];
   for (GLsizei si = 0; si < n; si++)
      p[si] = m_replay->MapProgramPipeline(p[si]);

   if (m_packet.NumItems() > 4)
      WarnDataNotHandled("glDeleteProgramPipelines");
   if (m_replay->GetDeviceCaps().m_has_GL_ES_VERSION_3_1 && m_replay->GetDispatch().real_glDeleteProgramPipelines)
      m_replay->GetDispatch().real_glDeleteProgramPipelines(n, p);
   else
      WarnNotAvailable("glDeleteProgramPipelines");
}

SPECIAL(glGenProgramPipelines)
{
   GLsizei numPPs = GetI32(GLsizei, 0);

   if (m_packet.NumItems() > 3)
      WarnDataNotHandled("glGenProgramPipelines");
   if (m_replay->GetDeviceCaps().m_has_GL_ES_VERSION_3_1 && m_replay->GetDispatch().real_glGenProgramPipelines)
      m_replay->GetDispatch().real_glGenProgramPipelines(numPPs, dummyGLuint);
   else
      WarnNotAvailable("glGenProgramPipelines");

   for (GLsizei si = 0; si < numPPs; si++)
   {
      if (m_retPacket.NumItems() > 2)
         m_replay->AddProgramPipelineMapping(dummyGLint[si], m_retPacket.Item(si + 2).GetUInt32());
      else
         m_replay->AddProgramPipelineMapping(dummyGLint[si], dummyGLint[si]);
   }
}

SPECIAL(glDispatchCompute)
{
   if (m_replay->SkipFrame(m_curFrame))
      return;

   if (m_replay->GetDeviceCaps().m_has_GL_ES_VERSION_3_1 && m_replay->GetDispatch().real_glDispatchCompute)
      m_replay->GetDispatch().real_glDispatchCompute(GetU32(GLuint, 0), GetU32(GLuint, 1), GetU32(GLuint, 2));
   else
      WarnNotAvailable("glDispatchCompute");
}

SPECIAL(glDispatchComputeIndirect)
{
   if (m_replay->SkipFrame(m_curFrame))
      return;

   if (m_replay->GetDeviceCaps().m_has_GL_ES_VERSION_3_1 && m_replay->GetDispatch().real_glDispatchComputeIndirect)
      m_replay->GetDispatch().real_glDispatchComputeIndirect(GetPtr(GLintptr, 0));
   else
      WarnNotAvailable("glDispatchComputeIndirect");
}

#endif // GL_ES_VERSION_3_0

#if GL_OES_mapbuffer
SPECIAL(glMapBufferOES)
{
   glMapBufferOES(GetU32(GLenum, 0), GetU32(GLenum, 1));
}

SPECIAL(glUnmapBufferOES)
{
   GLenum target = GetU32(GLenum, 0);
   uint32_t writing = GetU32(GLuint, 1);
   if (writing)
   {
      // GLint offset = GetI32(GLint, 2); unused
      GLint len = GetI32(GLint, 3);
      GLvoid     *ptr;
      GetArray(4, array[0]);
      glGetBufferPointervOES(target, GL_BUFFER_MAP_POINTER_OES, &ptr);

      if (ptr != NULL && array[0] != NULL)
         memcpy(ptr, array[0], len);
   }
   glUnmapBufferOES(target);
}

SPECIAL(glGetBufferPointervOES)
{
   glGetBufferPointervOES(GetU32(GLenum, 0), GetU32(GLenum, 1), array);
}
#else
// Emulate mapbuffer
SPECIAL(glMapBufferOES) {}
SPECIAL(glGetBufferPointervOES) {}

SPECIAL(glUnmapBufferOES)
{
   GLenum target = GetU32(GLenum, 0);
   uint32_t writing = GetU32(GLuint, 1);
   if (writing)
   {
      // GLint offset = GetI32(GLint, 2); unused
      GLint len = GetI32(GLint, 3);
      GetArray(4, array[0]);

      glBufferData(target, len, array[0], GL_DYNAMIC_DRAW);
   }
}
#endif
