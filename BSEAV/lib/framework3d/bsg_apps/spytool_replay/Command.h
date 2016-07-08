/*=============================================================================
Broadcom Proprietary and Confidential. (c)2012 Broadcom.
All rights reserved.

Project  :  PPP
Module   :  MMM

FILE DESCRIPTION
DESC
=============================================================================*/

#ifndef __COMMAND_H__
#define __COMMAND_H__

#define BSG_NO_NAME_MANGLING

#include "packet.h"
#if V3D_TECH_VERSION == 3
#include "api_command_ids.h"
#else
#include "interface/khronos/common/khrn_api_command_ids.h"
#endif

#include <GLES2/gl2.h>
#include <EGL/egl.h>

#include <map>
#include <string>

typedef void *FuncPtr;

class SpyToolReplay;

class DeferredVertexAttribPointer
{
public:
   DeferredVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean norm, GLsizei stride, const void *ptr,
                               GLuint boundBuffer, GLuint prog) :
      m_iptr(false), m_index(index), m_size(size), m_type(type), m_norm(norm), m_stride(stride), m_ptr(ptr), m_boundBuffer(boundBuffer), m_curProgram(prog) {}

   DeferredVertexAttribPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const void *ptr, GLuint boundBuffer, GLuint prog) :
      m_iptr(true), m_index(index), m_size(size), m_type(type), m_norm(false), m_stride(stride), m_ptr(ptr), m_boundBuffer(boundBuffer), m_curProgram(prog) {}

   GLboolean   m_iptr;
   GLuint      m_index;
   GLint       m_size;
   GLenum      m_type;
   GLboolean   m_norm;
   GLsizei     m_stride;
   const void *m_ptr;
   GLuint      m_boundBuffer;
   GLuint      m_curProgram;
};

class PerContextState
{
public:
   PerContextState() : m_curProgram(0) {}

   std::vector<DeferredVertexAttribPointer>  m_deferredVAPs;
   uint32_t                                  m_curProgram;
};

class Command
{
public:
   Command();
   ~Command();

   bool Execute(SpyToolReplay *replay, bool timing);   // Returns true if cmd was eglSwapBuffers
   void Clear();

   bool Valid() const { return m_packet.IsValid() &&
                              (m_packet.Type() == eREINIT || m_packet.Type() == eTHREAD_CHANGE || m_retPacket.IsValid()); }
   bool HasAPIFunc() const { return m_packet.IsValid(); }
   bool HasRetCode() const { return m_retPacket.IsValid(); }

   Packet &GetPacket() { return m_packet; }
   const Packet &GetPacket() const { return m_packet; }

   Packet &GetRetPacket() { return m_retPacket; }
   const Packet &GetRetPacket() const { return m_retPacket; }

   EGLConfig MatchConfig(uint32_t r, uint32_t g, uint32_t b, uint32_t a);
   EGLConfig MatchConfig(uint32_t r, uint32_t g, uint32_t b, uint32_t a, uint32_t d, uint32_t s, uint32_t samples);

   uint32_t &ByteSize() { return m_byteSize; }

   void AddDeleteItem(uint8_t *ptr) { m_deleteList.push_back(ptr); }

   void ProcessDeferredVAPs();

   // Overridden egl commands
   EGLBoolean eglSwapInterval(EGLDisplay dpy, EGLint interval);
   EGLBoolean eglMakeCurrent(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx);

   static void GetGLExtensions();
   static void WarnNotAvailable(const char *func);
   static void WarnDataNotHandled(const char *func);

protected:
   Packet                  m_packet;
   Packet                  m_retPacket;
   uint32_t                m_byteSize;
   std::vector<uint8_t *>  m_deleteList;
   SpyToolReplay           *m_replay;

   static uint32_t          m_curVPW;
   static uint32_t          m_curVPH;
   static EGLSurface        m_curDrawSurface;
   static EGLContext        m_delayedContextContext;
   static EGLDisplay        m_delayedContextDisplay;
   static EGLConfig         m_delayedContextConfig;
   static EGLContext        m_delayedContextShare;
   static uint32_t          m_delayedContextESVer;
   static EGLSurface        m_windowSurface;
   static EGLContext        m_curContext;

   static std::string       m_extensions;

   static std::map<EGLContext, PerContextState>  m_perContextState;
   static uint32_t                               m_curFrame;

   static EGLint           dummyEGLint[256];
   static EGLenum          dummyEGLenum[256];
   static GLuint           dummyGLuint[256];
   static GLint            dummyGLint[256];
   static GLsizei          dummyGLsizei[256];
#if GL_ES_VERSION_3_0
   static GLint64          dummyGLint64[256];
#endif
   static GLfloat          dummyGLfloat[256];
   static GLfixed          dummyGLfixed[256];
   static GLenum           dummyGLenum[256];
   static GLboolean        dummyGLboolean[256];
   static uint8_t          dummyvoid[256];
   static void             *dummyvoidptr[256];
   static EGLConfig        dummyEGLConfig[1024];
   static EGLSurface       dummyEGLSurface[256];
#if EGL_VERSION_1_5
   static EGLAttrib        dummyEGLAttrib[256];
#endif
   static void             *array[256];
   static GLchar            dummyGLchar[128 * 1024];

#include "CommandDecls.inc"
};

#define DECL_SPECIAL(func) virtual void run_##func();

class SpecializedCommand : public Command
{
public:
   DECL_SPECIAL(glCreateProgram)
   DECL_SPECIAL(glCreateShader)
   DECL_SPECIAL(glGetAttribLocation)
   DECL_SPECIAL(glGetUniformLocation)
   DECL_SPECIAL(glBindAttribLocation)
   DECL_SPECIAL(glClear)
   DECL_SPECIAL(glColorPointer)
   DECL_SPECIAL(glCompressedTexImage2D)
   DECL_SPECIAL(glCompressedTexSubImage2D)
   DECL_SPECIAL(glDisableVertexAttribArray)
   DECL_SPECIAL(glDrawArrays)
   DECL_SPECIAL(glDrawElements)
   DECL_SPECIAL(glEnableVertexAttribArray)
   DECL_SPECIAL(glGetActiveAttrib)
   DECL_SPECIAL(glNormalPointer)
   DECL_SPECIAL(glReadPixels)
   DECL_SPECIAL(glScissor)
   DECL_SPECIAL(glShaderBinary)
   DECL_SPECIAL(glShaderSource)
   DECL_SPECIAL(glLinkProgram)
   DECL_SPECIAL(glTexCoordPointer)
   DECL_SPECIAL(glTexImage2D)
   DECL_SPECIAL(glTexSubImage2D)
   DECL_SPECIAL(glUseProgram)
   DECL_SPECIAL(glVertexAttrib1f)
   DECL_SPECIAL(glVertexAttrib1fv)
   DECL_SPECIAL(glVertexAttrib2f)
   DECL_SPECIAL(glVertexAttrib2fv)
   DECL_SPECIAL(glVertexAttrib3f)
   DECL_SPECIAL(glVertexAttrib3fv)
   DECL_SPECIAL(glVertexAttrib4f)
   DECL_SPECIAL(glVertexAttrib4fv)
   DECL_SPECIAL(glVertexAttribPointer)
   DECL_SPECIAL(glVertexPointer)
   DECL_SPECIAL(glViewport)
   DECL_SPECIAL(glDrawTexsOES)
   DECL_SPECIAL(glDrawTexiOES)
   DECL_SPECIAL(glDrawTexxOES)
   DECL_SPECIAL(glDrawTexsvOES)
   DECL_SPECIAL(glDrawTexivOES)
   DECL_SPECIAL(glDrawTexxvOES)
   DECL_SPECIAL(glDrawTexfOES)
   DECL_SPECIAL(glDrawTexfvOES)
   DECL_SPECIAL(glPointSizePointerOES)
   DECL_SPECIAL(eglGetDisplay)
   DECL_SPECIAL(eglInitialize)
   DECL_SPECIAL(eglTerminate)
   DECL_SPECIAL(eglGetConfigAttrib)
   DECL_SPECIAL(eglGetConfigs)
   DECL_SPECIAL(eglChooseConfig)
   DECL_SPECIAL(eglCreateWindowSurface)
   DECL_SPECIAL(eglCreatePbufferSurface)
   DECL_SPECIAL(eglCreatePixmapSurface)
   DECL_SPECIAL(eglCreateContext)
   DECL_SPECIAL(eglMakeCurrent)
   DECL_SPECIAL(eglGetCurrentContext)
   DECL_SPECIAL(eglGetCurrentDisplay)
   DECL_SPECIAL(eglSwapBuffers)
   DECL_SPECIAL(eglCreateImageKHR)
   DECL_SPECIAL(eglCreateImage)
   DECL_SPECIAL(eglCreateSyncKHR)

   DECL_SPECIAL(glMapBufferOES)
   DECL_SPECIAL(glUnmapBufferOES)
   DECL_SPECIAL(glGetBufferPointervOES)

   DECL_SPECIAL(glRenderbufferStorage)

#if GL_ES_VERSION_3_0
   DECL_SPECIAL(glDrawRangeElements)
   DECL_SPECIAL(glTexImage3D)
   DECL_SPECIAL(glTexSubImage3D)
   DECL_SPECIAL(glCompressedTexImage3D)
   DECL_SPECIAL(glCompressedTexSubImage3D)
   DECL_SPECIAL(glGenQueries)
   DECL_SPECIAL(glDeleteQueries)
   DECL_SPECIAL(glUnmapBuffer)
   DECL_SPECIAL(glDrawBuffers)
   DECL_SPECIAL(glFlushMappedBufferRange)
   DECL_SPECIAL(glDeleteVertexArrays)
   DECL_SPECIAL(glGenVertexArrays)
   DECL_SPECIAL(glTransformFeedbackVaryings)
   DECL_SPECIAL(glGetTransformFeedbackVarying)
   DECL_SPECIAL(glVertexAttribIPointer)
   DECL_SPECIAL(glGetVertexAttribIiv)
   DECL_SPECIAL(glGetVertexAttribIuiv)
   DECL_SPECIAL(glVertexAttribI4i)
   DECL_SPECIAL(glVertexAttribI4ui)
   DECL_SPECIAL(glVertexAttribI4iv)
   DECL_SPECIAL(glVertexAttribI4uiv)
   DECL_SPECIAL(glGetUniformIndices)
   DECL_SPECIAL(glGetActiveUniformsiv)
   DECL_SPECIAL(glDrawArraysInstanced)
   DECL_SPECIAL(glDrawElementsInstanced)
   DECL_SPECIAL(glFenceSync)
   DECL_SPECIAL(glClientWaitSync)
   DECL_SPECIAL(glWaitSync)
   DECL_SPECIAL(glGetSynciv)
   DECL_SPECIAL(glGenSamplers)
   DECL_SPECIAL(glDeleteSamplers)
   DECL_SPECIAL(glDeleteTransformFeedbacks)
   DECL_SPECIAL(glGenTransformFeedbacks)
   DECL_SPECIAL(glGetProgramBinary)
   DECL_SPECIAL(glDrawArraysIndirect)
   DECL_SPECIAL(glDrawElementsIndirect)
   DECL_SPECIAL(glCreateShaderProgramv)
   DECL_SPECIAL(glDeleteProgramPipelines)
   DECL_SPECIAL(glGenProgramPipelines)
#endif // GL_ES_VERSION_3_0
};

#endif /* __COMMAND_H__ */
