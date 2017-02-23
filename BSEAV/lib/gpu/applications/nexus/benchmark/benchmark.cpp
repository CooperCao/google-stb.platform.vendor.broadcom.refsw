/***************************************************************************
 *     Broadcom Proprietary and Confidential. (c)2010 Broadcom.  All rights reserved.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 *
 **************************************************************************/

#include <math.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <malloc.h>
#include <assert.h>
#include <string.h>
#include <alloca.h>
#include <sys/time.h>

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include "load_texture.h"
#include "esutil.h"
#include "../common/init.h"

#include "default_nexus.h"

NEXUS_DISPLAYHANDLE  nexus_display = 0;
EGLNativeDisplayType native_display = 0;
void                 *native_window = 0;

NXPL_PlatformHandle nxpl_handle = 0;

const unsigned int WIDTH             = 1280;
const unsigned int HEIGHT            = 720;
const unsigned int BPP               = 16;

struct GLFormatInfo
{
   GLenum   m_internalFormat;
   GLenum   m_format;
   GLenum   m_type;
   GLuint   m_size;
};

typedef enum
{
   MODE_NORMAL,
   MODE_TEXTURE_LOAD,
   MODE_TEXTURE_FILTER,
   MODE_NONE = 0xffffffff
} Mode_t;

typedef enum
{
   TEX_FMT_RGBA8888,
   TEX_FMT_RGB888,
   TEX_FMT_RGB565,
   TEX_FMT_ETC1,
   TEX_FMT_LA88,
   TEX_FMT_A8,
   TEX_FMT_L8
} TexFmt_t;

static unsigned int getTimeNowMs(void)
{
   struct timeval curTime;
   unsigned int nowMs;

   gettimeofday(&curTime, NULL);
   nowMs = curTime.tv_usec / 1000;
   nowMs += curTime.tv_sec * 1000;

   return nowMs;
}

///////////////////////////////////////////////////////////////////////////////
// Error handling
///////////////////////////////////////////////////////////////////////////////
void Error(const char *msg, const char *extra)
{
   std::cout << "ERROR: " << msg << extra << "\n";
   throw msg;
}

const char *GLErrToString(GLenum err)
{
   const char  *ret = "Unknown error";

   switch (err)
   {
   case GL_NO_ERROR:
      ret = "No error";
      break;

   case GL_INVALID_ENUM:
      ret = "Invalid enum";
      break;

   case GL_INVALID_VALUE:
      ret = "Invalid value";
      break;

   case GL_INVALID_OPERATION:
      ret = "Invalid operation";
      break;

   case GL_OUT_OF_MEMORY:
      ret = "Out of memory";
      break;
   }

   return ret;
}

std::ostream &operator<<(std::ostream &os, TexFmt_t f)
{
   switch (f)
   {
   case TEX_FMT_RGBA8888:
      os << "rgba8888";
      break;

   case TEX_FMT_RGB888:
      os << "rgb888";
      break;

   case TEX_FMT_RGB565:
      os << "rgb565";
      break;

   case TEX_FMT_ETC1:
      os << "etc1";
      break;

   case TEX_FMT_LA88:
      os << "la88";
      break;

   case TEX_FMT_A8:
      os << "a8";
      break;

   case TEX_FMT_L8:
      os << "l8";
      break;
   }

   return os;
}

const char *ModeToString(GLenum mode)
{
   const char *ret;

   switch (mode)
   {
   case GL_NEAREST:
      ret = "nearest no mipmap";
      break;
   case GL_LINEAR:
      ret = "linear no mipmap";
      break;
   case GL_NEAREST_MIPMAP_NEAREST:
      ret = "nearest with mipmap";
      break;
   case GL_LINEAR_MIPMAP_NEAREST:
      ret = "linear with mipmap";
      break;
   case GL_LINEAR_MIPMAP_LINEAR:
      ret = "trilinear";
      break;
   default:
      ret = "unknown";
      break;
   }

   return ret;
}

unsigned int ImageDestFormat(TexFmt_t fmt)
{
   unsigned int ret = 0;

   switch (fmt)
   {
   case TEX_FMT_RGBA8888:
      ret = DF_8888;
      break;

   case TEX_FMT_RGB888:
      ret = DF_888;
      break;

   case TEX_FMT_RGB565:
      ret = DF_565;
      break;

   default:
      Error("ImageDestFormat passed invalid texture format", "");
      break;
   }

   return ret;
}

GLFormatInfo GetFormatInfo(TexFmt_t fmt)
{
   GLFormatInfo   res;

   switch (fmt)
   {
   case TEX_FMT_RGBA8888:
      res.m_internalFormat = GL_RGBA;
      res.m_format         = GL_RGBA;
      res.m_type           = GL_UNSIGNED_BYTE;
      res.m_size           = 4;
      break;

   case TEX_FMT_RGB888:
      res.m_internalFormat = GL_RGB;
      res.m_format         = GL_RGB;
      res.m_type           = GL_UNSIGNED_BYTE;
      res.m_size           = 3;
      break;

   case TEX_FMT_RGB565:
      res.m_internalFormat = GL_RGB;
      res.m_format         = GL_RGB;
      res.m_type           = GL_UNSIGNED_SHORT_5_6_5;
      res.m_size           = 2;
      break;

   case TEX_FMT_ETC1:
      res.m_internalFormat = GL_ETC1_RGB8_OES;
      res.m_format         = 0;
      res.m_type           = 0;
      res.m_size           = 1; // More than enough!
      break;

   case TEX_FMT_LA88:
      res.m_internalFormat = GL_LUMINANCE_ALPHA;
      res.m_format         = GL_LUMINANCE_ALPHA;
      res.m_type           = GL_UNSIGNED_BYTE;
      res.m_size           = 2;
      break;

   case TEX_FMT_A8:
      res.m_internalFormat = GL_ALPHA;
      res.m_format         = GL_ALPHA;
      res.m_type           = GL_UNSIGNED_BYTE;
      res.m_size           = 1;
      break;

   case TEX_FMT_L8:
      res.m_internalFormat = GL_LUMINANCE;
      res.m_format         = GL_LUMINANCE;
      res.m_type           = GL_UNSIGNED_BYTE;
      res.m_size           = 1;
      break;
   }

   return res;
}


///////////////////////////////////////////////////////////////////////////////
// Options class
//
// All run-time options are held in this class
///////////////////////////////////////////////////////////////////////////////
struct Options
{
   Options()
   {
      Init();
   }

   Options(const Options &rhs) :
      m_mode              (rhs.m_mode              ),
      m_rectWidthInPixels (rhs.m_rectWidthInPixels ),
      m_rectHeightInPixels(rhs.m_rectHeightInPixels),
      m_numRectsInGrid    (rhs.m_numRectsInGrid    ),
      m_numDrawRepeats    (rhs.m_numDrawRepeats    ),
      m_numSampleFrames   (rhs.m_numSampleFrames   ),
      m_numItersPerFrame  (rhs.m_numItersPerFrame  ),
      m_useVBO            (rhs.m_useVBO            ),
      m_useTransform      (rhs.m_useTransform      ),
      m_useIndexArray     (rhs.m_useIndexArray     ),
      m_useDepthTest      (rhs.m_useDepthTest      ),
      m_useVaryingColor   (rhs.m_useVaryingColor   ),
      m_useVaryingDepth   (rhs.m_useVaryingDepth   ),
      m_useFrontToBack    (rhs.m_useFrontToBack    ),
      m_useStrip          (rhs.m_useStrip          ),
      m_useBlend          (rhs.m_useBlend          ),
      m_positionSize      (rhs.m_positionSize      ),
      m_texFile           (rhs.m_texFile           ),

      m_textureLoad       (rhs.m_textureLoad       ),
      m_textureLoadWidth  (rhs.m_textureLoadWidth  ),
      m_textureLoadHeight (rhs.m_textureLoadHeight ),
      m_textureLoadBits   (rhs.m_textureLoadBits   ),

      m_textureFilter     (rhs.m_textureFilter     ),
      m_textureFilterMode (rhs.m_textureFilterMode ),
      m_textureFilterBits (rhs.m_textureFilterBits ),
      m_textureFilterLvls (rhs.m_textureFilterLvls ),

      m_batchFile         (rhs.m_batchFile         ),
      m_outFile           (rhs.m_outFile           ),
      m_useMultisample    (rhs.m_useMultisample    ),
      m_usePreservingSwap (rhs.m_usePreservingSwap ),
      m_stretchToFit      (rhs.m_stretchToFit      ),
      m_useDepth          (rhs.m_useDepth          ),
      m_useStencil        (rhs.m_useStencil        ),
      m_vpW               (rhs.m_vpW               ),
      m_vpH               (rhs.m_vpH               ),
      m_bpp               (rhs.m_bpp               ),
      m_showVersion       (rhs.m_showVersion       ),

      m_clientId          (rhs.m_clientId          )
   {
   }

   void Init()
   {
      m_mode               = MODE_NORMAL;
      m_rectWidthInPixels  = 1.0f;
      m_rectHeightInPixels = 1.0f;
      m_numRectsInGrid     = 10000;
      m_numDrawRepeats     = 1;
      m_numSampleFrames    = 10;
      m_numItersPerFrame   = 1;
      m_useVBO             = true;
      m_useTransform       = false;
      m_useIndexArray      = true;
      m_useDepthTest       = false;
      m_useVaryingColor    = false;
      m_useVaryingDepth    = false;
      m_useFrontToBack     = false;
      m_useStrip           = true;
      m_useBlend           = false;
      m_positionSize       = 3;
      m_texFile            = "";

      m_textureLoad        = 0;
      m_textureLoadWidth   = 1024;
      m_textureLoadHeight  = 1024;
      m_textureLoadBits    = TEX_FMT_RGBA8888;

      m_textureFilter      = 0;
      m_textureFilterMode  = GL_NEAREST;
      m_textureFilterBits  = TEX_FMT_RGBA8888;
      m_textureFilterLvls  = 1;

      m_batchFile          = "";
      m_outFile            = "";
      m_useMultisample     = false;
      m_usePreservingSwap  = false;
      m_stretchToFit       = true;
      m_useDepth           = true;
      m_useStencil         = true;
      m_vpW                = WIDTH;
      m_vpH                = HEIGHT;
      m_bpp                = BPP;

      m_showVersion        = false;
      m_clientId           = 0;
   }

   void Validate()
   {
      if (m_useVaryingDepth && m_useIndexArray)
         Error("Varying the depth is inconsistent with using index arrays i.e. depth will not vary", "");
   }

   void ParseLine(const std::string &str)
   {
      unsigned int ParseArg(const char *arg, Options *options);

      std::stringstream ss(str);
      std::string       arg;

      while (ss >> arg)
      {
         if (!ParseArg(arg.c_str(), this))
            Error("Argument not recognised: ", arg.c_str());
      }

      Validate();
   }

   void DumpHeader(std::ostream &os) const
   {
      switch (m_mode)
      {
      case MODE_NORMAL:
         os << "Rect X, Rect Y, ";
         os << "Rect Number, ";
         os << "Draw Repeats, ";
         os << "Frame Repeats, ";
         os << "Iters per Frame, ";
         os << "VBO, ";
         os << "Transform, ";
         os << "Index, ";
         os << "Depth, ";
         os << "Varying Color, ";
         os << "Varying Depth, ";
         os << "FrontToBack, ";
         os << "Strip, ";
         os << "Blend, ";
         os << "Position elements, ";
         os << "Texture";
         break;
      case MODE_TEXTURE_LOAD:
         os << "Texture width, Texture height, Texture bits";
         break;
      case MODE_TEXTURE_FILTER:
         os << "Repeats, Frames, Mode, Width, Height, Format, File, Levels, Blend";
         break;
      default:
         os << "Error: unknown mode";
         break;
      }
   }

   void Dump(std::ostream &os) const
   {
      switch (m_mode)
      {
      case MODE_NORMAL:
         os << m_rectWidthInPixels << ", " << m_rectHeightInPixels << ", ";
         os << m_numRectsInGrid    << ", ";
         os << m_numDrawRepeats    << ", ";
         os << m_numSampleFrames   << ", ";
         os << m_numItersPerFrame  << ", ";
         os << m_useVBO            << ", ";
         os << m_useTransform      << ", ";
         os << m_useIndexArray     << ", ";
         os << m_useDepthTest      << ", ";
         os << m_useVaryingColor   << ", ";
         os << m_useVaryingDepth   << ", ";
         os << m_useFrontToBack    << ", ";
         os << m_useStrip          << ", ";
         os << m_useBlend          << ", ";
         os << m_positionSize      << ", ";
         os << m_texFile;
         break;
      case MODE_TEXTURE_LOAD:
         os << m_textureLoadWidth << ", " << m_textureLoadHeight << ", ";
         os << m_textureLoadBits;
         break;
      case MODE_TEXTURE_FILTER:
         os << m_textureFilter << ", " << m_numSampleFrames << ", ";
         os << ModeToString(m_textureFilterMode) << ", ";
         os << m_rectWidthInPixels << ", " << m_rectHeightInPixels << ", ";
         os << m_textureFilterBits << ", ";
         if (m_texFile != "")
            os << m_texFile;
         else
            os << "No texture";
         os << ", " << m_textureFilterLvls;
         os << ", " << m_useBlend;
         break;
      default:
         os << "Error: unknown mode";
         break;
      }
   }

   bool UseTexture()   const  { return m_texFile   != ""; }
   bool UseBatchMode() const  { return m_batchFile != ""; }

   GLenum MagFilterMode() const
   {
      return GL_LINEAR;
   }

   GLenum MinFilterMode() const
   {
      return m_textureFilterMode;
   }

   Mode_t       m_mode;
   float        m_rectWidthInPixels;
   float        m_rectHeightInPixels;
   int          m_numRectsInGrid;
   int          m_numDrawRepeats;
   int          m_numSampleFrames;
   int          m_numItersPerFrame;
   bool         m_useVBO;
   bool         m_useTransform;
   bool         m_useIndexArray;
   bool         m_useDepthTest;
   bool         m_useVaryingColor;
   bool         m_useVaryingDepth;
   bool         m_useFrontToBack;
   bool         m_useStrip;
   bool         m_useBlend;
   int          m_positionSize;
   std::string  m_texFile;

   unsigned int m_textureLoad;
   unsigned int m_textureLoadWidth;
   unsigned int m_textureLoadHeight;
   TexFmt_t     m_textureLoadBits;

   unsigned int m_textureFilter;
   GLenum       m_textureFilterMode;
   TexFmt_t     m_textureFilterBits;
   unsigned int m_textureFilterLvls;

   std::string  m_batchFile;
   std::string  m_outFile;
   bool         m_useMultisample;
   bool         m_usePreservingSwap;
   bool         m_stretchToFit;
   bool         m_useDepth;
   bool         m_useStencil;
   int          m_vpW;
   int          m_vpH;
   int          m_bpp;
   bool         m_showVersion;
   unsigned     m_clientId;
};

///////////////////////////////////////////////////////////////////////////////
// Program class
//
// Generates shader program according to the options
///////////////////////////////////////////////////////////////////////////////
class Program
{
public:
   Program(const Options &options);
   ~Program();

   void SetPosition(GLuint size, GLenum type, GLsizei stride, void *data) const;
   void SetTexCoord(GLuint size, GLenum type, GLsizei stride, void *data) const;
   void SetMatrix(const ESMatrix &matrix) const;
   void SetTex(GLuint unit) const;
   void SetColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a) const;
   void SetDepth(GLfloat d) const;

private:
   GLuint    m_prog;
   GLuint    m_vert;
   GLuint    m_frag;

   GLint     m_position;
   GLint     m_tc;
   GLint     m_matrix;
   GLint     m_tex;
   GLint     m_color;
   GLint     m_depth;
};


static void Add(std::string &s, const char *line)
{
   s += line;
   s += "\n";
}

static std::string MakeVShader(const Options &options)
{
   std::string s;

   if (options.m_useVaryingDepth)
   {
      Add(s, "uniform float   u_depth;");
   }

   Add(s, "attribute vec4 a_position;");

   if (options.UseTexture())
   {
      Add(s, "attribute vec2   a_tc;");
      Add(s, "varying   vec2   v_tc;");
   }

   if (options.m_useTransform)
      Add(s, "uniform mat4 u_matrix;");

   Add(s, "void main()");
   Add(s, "{");

   if (options.UseTexture())
      Add(s, "   v_tc = a_tc;");

   if (!options.m_useTransform)
   {
      if (options.m_useVaryingDepth)
         Add(s, "   gl_Position = vec4(a_position.xy, u_depth, 1.0);");
      else
         Add(s, "   gl_Position = a_position;");
   }
   else
   {
      if (options.m_useVaryingDepth)
         Add(s, "   gl_Position = u_matrix * vec4(a_position.xy, u_depth, 1.0);");
      else
         Add(s, "   gl_Position = u_matrix * a_position;");
   }

   Add(s,  "}");

   return s;
}

static std::string MakeFShader(const Options &options)
{
   std::string s;

   Add(s, "precision mediump float;");

   if (options.m_useVaryingColor)
      Add(s, "uniform vec4 u_color;");

   if (options.UseTexture())
   {
      Add(s, "uniform sampler2D u_tex;");
      Add(s, "varying vec2   v_tc;");
   }

   Add(s, "void main()");
   Add(s, "{");

   if (options.UseTexture())
   {
      if (options.m_useVaryingColor)
         Add(s, "   gl_FragColor = texture2D(u_tex, v_tc) * u_color;");
      else
         Add(s, "   gl_FragColor = texture2D(u_tex, v_tc);");
   }
   else
   {
      if (options.m_useVaryingColor)
         Add(s, "   gl_FragColor = u_color;");
      else
         Add(s, "   gl_FragColor = vec4(1.0, 0.3, 0.2, 1.0);");
   }

   Add(s, "}");

   return s;
}

/* CheckCompile, CheckLink,
 *
 * Helper functions for compilation process
 */
static void CheckCompile(const char *name, GLint shader)
{
   GLint ret;

   glGetShaderiv(shader, GL_COMPILE_STATUS, &ret);
   if (ret == GL_FALSE)
   {
      glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &ret);
      std::vector<char> err(ret);
      glGetShaderInfoLog(shader, ret, NULL, &err[0]);
      std::cout << "Shader '" << name << "' compile failed: " << &err[0] << "\n";
      assert(0);
   }
}

static void CheckLink(const char *name, GLint program)
{
   GLint ret;

   glGetProgramiv(program, GL_LINK_STATUS, &ret);
   if (ret == GL_FALSE)
   {
      glGetProgramiv(program, GL_INFO_LOG_LENGTH, &ret);
      std::vector<char> err(ret);
      glGetProgramInfoLog(program, ret, NULL, &err[0]);
      std::cout << "Program '" << name << "' link failed: " << &err[0] << "\n";
      assert(0);
   }
}

static void ShaderSource(GLuint shader, const char *src)
{
   //std::cout << "Source Code:\n\n" << src << "\n";

   glShaderSource(shader, 1, &src, NULL);
}

/* InitShaders
 *
 * Compile shaders and link
 */
Program::Program(const Options &options) :
   m_position(-1),
   m_tc(-1),
   m_matrix(-1),
   m_tex(-1),
   m_color(-1)
{
   m_vert = glCreateShader(GL_VERTEX_SHADER);
   m_frag = glCreateShader(GL_FRAGMENT_SHADER);

   ShaderSource(m_vert, MakeVShader(options).c_str());
   ShaderSource(m_frag, MakeFShader(options).c_str());

   glCompileShader(m_vert);
   CheckCompile("vertex shader", m_vert);

   glCompileShader(m_frag);
   CheckCompile("fragment shader", m_frag);

   m_prog = glCreateProgram();
   glAttachShader(m_prog, m_vert);
   glAttachShader(m_prog, m_frag);

   glLinkProgram(m_prog);
   CheckLink("program", m_prog);

   /* Get the attribute locations */
   m_position = glGetAttribLocation(m_prog, "a_position");
   m_tc       = glGetAttribLocation(m_prog, "a_tc");

   /* Get the uniform locations */
   m_matrix = glGetUniformLocation(m_prog, "u_matrix");
   m_tex    = glGetUniformLocation(m_prog, "u_tex");
   m_color  = glGetUniformLocation(m_prog, "u_color");
   m_depth  = glGetUniformLocation(m_prog, "u_depth");

   glUseProgram(m_prog);
}

Program::~Program()
{
   if (m_position != -1)
      glDisableVertexAttribArray(m_position);
   if (m_tc != -1)
      glDisableVertexAttribArray(m_tc);
   glDeleteProgram(m_prog);
   glDeleteShader(m_vert);
   glDeleteShader(m_frag);
}

void Program::SetPosition(GLuint size, GLenum type, GLsizei stride, void *data) const
{
   if (m_position != -1)
   {
      glEnableVertexAttribArray(m_position);
      glVertexAttribPointer(m_position, size, type, GL_FALSE, stride, data);
   }
}

void Program::SetTexCoord(GLuint size, GLenum type, GLsizei stride, void *data) const
{
   if (m_tc != -1)
   {
      glEnableVertexAttribArray(m_tc);
      glVertexAttribPointer(m_tc, size, type, GL_FALSE, stride, data);
   }
}

void Program::SetMatrix(const ESMatrix &matrix) const
{
   if (m_matrix != -1)
      glUniformMatrix4fv(m_matrix, 1, GL_FALSE, (GLfloat*) &matrix.m[0][0]);
}

void Program::SetTex(GLuint unit) const
{
   if (m_tex != -1)
      glUniform1i(m_tex, unit);
}

void Program::SetColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a) const
{
   if (m_color != -1)
      glUniform4f(m_color, r, g, b, a);
}

void Program::SetDepth(GLfloat d) const
{
   if (m_depth != -1)
      glUniform1f(m_depth, d);
}

///////////////////////////////////////////////////////////////////////////////
// GL Buffer class
///////////////////////////////////////////////////////////////////////////////
class GLBuffer
{
public:
   GLBuffer(GLenum type) :
      m_type(type)
   {
      glGenBuffers(1, &m_id);
   }

   ~GLBuffer()
   {
      glDeleteBuffers(1, &m_id);
   }

   void Bind() const
   {
      glBindBuffer(m_type, m_id);
   }

   void Data(GLsizei size, void *data)
   {
      glBufferData(m_type, size, data, GL_STATIC_DRAW);
   }

private:
   GLuint   m_id;
   GLenum   m_type;
};

///////////////////////////////////////////////////////////////////////////////
// Global variables
///////////////////////////////////////////////////////////////////////////////

static Options g_options;

///////////////////////////////////////////////////////////////////////////////
// Geometry generation
///////////////////////////////////////////////////////////////////////////////
float *EmitVertex(float x, float y, float *arr, int elems)
{
   *arr++ = x;
   *arr++ = y;
   if (elems > 2)
      *arr++ = 0.0f;
   if (elems > 3)
      *arr++ = 1.0f;

   return arr;
}

float *EmitTC(float i, float j, float *arr, bool useTexture)
{
   if (useTexture)
   {
      *arr++ = i;
      *arr++ = j;
   }

   return arr;
}

std::vector<float> MakeGridStrip(const Options &options, int &numVerts, int &floatsPerVert)
{
   int   rects  = options.m_numRectsInGrid;
   float w      = options.m_rectWidthInPixels;
   float h      = options.m_rectHeightInPixels;
   int   elems  = options.m_positionSize;
   bool  useTex = options.UseTexture();

   int   side  = (int)ceil(sqrt((double)rects));

   // Each row adds 4 vertices (two to start off and two to link to the next and previous
   // rows with degenerates)
   // Each rectangle adds two vertices
   // There is no linking vertex at the start of end of the draw
   numVerts      = side * 4 + rects * 2;
   floatsPerVert = elems + (useTex ? 2 : 0);

   if (numVerts > 0xffff)
      Error("Too many vertices in grid", "");

   std::vector<float>   pos(numVerts * floatsPerVert);

   int   v, x, y;

   for (v = 0, y = 0; y < side && v < numVerts; ++y)
   {
      for (x = 0; x <= side && v < numVerts; ++x)
      {
         float    factorW = 2.0f / options.m_vpW;
         float    factorH = 2.0f / options.m_vpH;

         float    x0 = w * factorW * x - 1.0f;
         float    y0 = h * factorH * y - 1.0f;
         float    y1 = y0 + h * factorH;

         float    i0 = (float)x / side;
         float    j0 = (float)y / side;
         float    j1 = (float)(y + 1) / side;

         float    *ptr = &pos[v * floatsPerVert];

         // Repeat first vertex even on first row (so strips can be concatenated)
         if (x == 0)
         {
            ptr = EmitVertex(x0, y0, ptr, elems);
            ptr = EmitTC(i0, j0, ptr, useTex);
            v += 1;
         }

         ptr = EmitVertex(x0, y0, ptr, elems);
         ptr = EmitTC(i0, j0, ptr, useTex);
         v += 1;

         ptr = EmitVertex(x0, y1, ptr, elems);
         ptr = EmitTC(i0, j1, ptr, useTex);
         v += 1;

         // Repeat last vertex even on last row (so strips can be concatenated)
         if (x == side)
         {
            ptr = EmitVertex(x0, y1, ptr, elems);
            ptr = EmitTC(i0, j1, ptr, useTex);
            v += 1;
         }
      }
   }

   return pos;
}

std::vector<float> MakeGridTriangles(const Options &options, int &numVerts, int &floatsPerVert)
{
   static const int  VERTS_PER_RECT = 6;

   int   rects  = options.m_numRectsInGrid;
   float w      = options.m_rectWidthInPixels;
   float h      = options.m_rectHeightInPixels;
   int   elems  = options.m_positionSize;
   bool  useTex = options.UseTexture();
   int   side   = (int)ceil(sqrt((double)rects));

   floatsPerVert = elems + (useTex ? 2 : 0);
   numVerts      = VERTS_PER_RECT * rects;

   if (numVerts > 0xffff)
      Error("Too many vertices in grid", "");

   std::vector<float>   pos(numVerts * floatsPerVert);

   int   r, x, y;

   for (r = 0, y = 0; y < side && r < rects; ++y)
   {
      for (x = 0; x < side && r < rects; ++x, ++r)
      {
         float    factorW = 2.0f / options.m_vpW;
         float    factorH = 2.0f / options.m_vpH;

         float    x0 = w * factorW * x - 1.0f;
         float    y0 = h * factorH * y - 1.0f;
         float    x1 = x0 + w * factorW;
         float    y1 = y0 + h * factorH;

         float    i0 = (float)x / side;
         float    j0 = (float)y / side;
         float    i1 = (float)(x + 1) / side;
         float    j1 = (float)(y + 1) / side;

         float    *ptr = &pos[r * floatsPerVert * VERTS_PER_RECT];

         ptr = EmitVertex(x0, y0, ptr, elems);
         ptr = EmitTC(i0, j0, ptr, useTex);
         ptr = EmitVertex(x0, y1, ptr, elems);
         ptr = EmitTC(i0, j1, ptr, useTex);
         ptr = EmitVertex(x1, y0, ptr, elems);
         ptr = EmitTC(i1, j0, ptr, useTex);

         ptr = EmitVertex(x1, y0, ptr, elems);
         ptr = EmitTC(i1, j0, ptr, useTex);
         ptr = EmitVertex(x0, y1, ptr, elems);
         ptr = EmitTC(i0, j1, ptr, useTex);
         ptr = EmitVertex(x1, y1, ptr, elems);
         ptr = EmitTC(i1, j1, ptr, useTex);
      }
   }

   return pos;
}

std::vector<GLushort> MakeIndexArray(int verts, int repeats)
{
   std::vector<GLushort>   index(verts * repeats);

   int   i = 0;

   for (int repeat = 0; repeat < repeats; ++repeat)
   {
      for (int vert = 0; vert < verts; ++vert)
      {
         index[i] = vert;

         // Check that the last parts of the index array are actually used
         //if (repeat != repeats - 1 || vert < 1000)
         //   index[i] = 0;

         i += 1;
      }
   }

   return index;
}

///////////////////////////////////////////////////////////////////////////////
// CountPixels -- use to verify that pixels drawn correlates with expected area
///////////////////////////////////////////////////////////////////////////////
int CountPixels(GLubyte r, GLubyte g, GLubyte b)
{
   std::vector<GLubyte> pixels(4 * WIDTH * HEIGHT);

   glReadPixels(0, 0, WIDTH, HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, &pixels[0]);

   int   count = 0;

   for (unsigned int p = 0; p < WIDTH * HEIGHT; ++p)
   {
      if (pixels[p * 4] == r && pixels[p * 4 + 1] == g && pixels[p * 4 + 2] == b)
         count += 1;
   }

   return count;
}

void LoadPackedTexture(const char *baseName)
{
   std::string fileName = baseName;
   fileName += ".pkm";

   PKMImage image;
   LoadPKM(fileName.c_str(), &image);

   if (image.mBytes == 0)
      Error("Texture not found: ", fileName.c_str());

   glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_ETC1_RGB8_OES, image.mWidth, image.mHeight, 0, image.mSize, image.mBytes);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   FreePKM(&image);
}

void LoadFilterTexturePacked(const char *baseName, TexFmt_t fmt, GLenum magFilterMode, GLenum minFilterMode, GLuint numLevels)
{
   GLFormatInfo   info       = GetFormatInfo(fmt);
   std::string    fileName   = baseName;

   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

   for (unsigned int i = 0; i < numLevels; ++i)
   {
      std::stringstream  ss;
      ss << fileName << i << ".pkm";

      PKMImage image;
      std::string file = ss.str();

      if (!LoadPKM(file.c_str(), &image))
         Error("Texture not found: ", file.c_str());

      glCompressedTexImage2D(GL_TEXTURE_2D, i, GL_ETC1_RGB8_OES, image.mWidth, image.mHeight, 0, image.mSize, image.mBytes);
      FreePKM(&image);
   }

   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilterMode);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilterMode);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void LoadFilterTexture(const char *baseName, TexFmt_t fmt, GLenum magFilterMode, GLenum minFilterMode, GLuint numLevels)
{
   int            destFormat = ImageDestFormat(fmt);
   GLFormatInfo   info       = GetFormatInfo(fmt);
   std::string    fileName   = baseName;

   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

   for (unsigned int i = 0; i < numLevels; ++i)
   {
      std::stringstream  ss;
      ss << fileName << i << ".png";

      PNGImage image;
      std::string file = ss.str();

      if (!LoadPNG(file.c_str(), destFormat, &image))
      {
         Error("Texture not found: ", file.c_str());
      }
      else
      {
         glTexImage2D(GL_TEXTURE_2D, i, info.m_internalFormat, image.mWidth, image.mHeight, 0, info.m_format, info.m_type, image.mBytes);
         FreePNG(&image);
      }
   }

   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilterMode);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilterMode);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void InitGLState(const Options &options)
{
   glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
   glClearDepthf(1.0f);
   glClearStencil(0);

   glViewport(0, 0, options.m_vpW, options.m_vpH);

   if (options.m_useDepthTest || options.m_useVaryingDepth)
   {
      glDepthFunc(options.m_useVaryingDepth ? GL_LESS : GL_ALWAYS);
      glEnable(GL_DEPTH_TEST);
   }
   else
   {
      glDisable(GL_DEPTH_TEST);
   }

   if (options.m_useBlend)
   {
      glBlendFunc(GL_ONE_MINUS_SRC_COLOR, GL_CONSTANT_COLOR);
      glBlendColor(0.5f, 0.7f, 0.5f, 1.0f);
      glEnable(GL_BLEND);
   }
   else
   {
      glDisable(GL_BLEND);
   }

   if (options.UseTexture())
      LoadPackedTexture(options.m_texFile.c_str());
}

float UpdateDrawColor(const Options &options, const Program &prog, float color)
{

   if (!options.m_useVaryingColor)
      return 0.0f;

   prog.SetColor(color, color, color, 1.0f);

   float ret = color + 0.1f;
   return ret > 1.0f ? 0.1f : ret;
}

///////////////////////////////////////////////////////////////////////////////
// RunTest
//
// Runs the test specified by options and logs results to the stream
///////////////////////////////////////////////////////////////////////////////
void RunRenderTest(const Options &options, std::ostream &os)
{
   const GLenum   mode = options.m_useStrip ? GL_TRIANGLE_STRIP : GL_TRIANGLES;

   ESMatrix       transform;
   esMatrixLoadIdentity(&transform);

   Program     prog(options);
   prog.SetMatrix(transform);
   prog.SetTex(0);

   int                     numVerts;
   int                     floatsPerVert;

   std::vector<GLushort>   indices;
   std::vector<float>      vertices = options.m_useStrip ? MakeGridStrip(options, numVerts, floatsPerVert) :
                                                           MakeGridTriangles(options, numVerts, floatsPerVert);

   GLsizei                 stride = floatsPerVert * sizeof(GLfloat);

   if (options.m_useIndexArray)
      indices = MakeIndexArray(numVerts, options.m_numDrawRepeats);

   InitGLState(options);

   GLBuffer   vertexBuffer(GL_ARRAY_BUFFER);
   GLBuffer   indexBuffer(GL_ELEMENT_ARRAY_BUFFER);

   if (options.m_useVBO)
   {
      vertexBuffer.Bind();
      vertexBuffer.Data(vertices.size() * sizeof(GLfloat), &vertices[0]);

      prog.SetPosition(options.m_positionSize, GL_FLOAT, stride, 0);
      if (options.UseTexture())
         prog.SetTexCoord(2, GL_FLOAT, stride, (void *)(sizeof(GLfloat) * options.m_positionSize));

      if (options.m_useIndexArray)
      {
         indexBuffer.Bind();
         indexBuffer.Data(indices.size() * sizeof(GLushort), &indices[0]);
      }
   }
   else
   {
      prog.SetPosition(options.m_positionSize, GL_FLOAT, stride, &vertices[0]);
      if (options.UseTexture())
         prog.SetTexCoord(2, GL_FLOAT, stride, &vertices[options.m_positionSize]);
   }

   // Clear any errors
   glGetError();

   //  Start timing
   int   t0    = getTimeNowMs();
   float color = 0.1f;

   // Main rendering loop
   for (int f = 0; f < options.m_numSampleFrames; ++f)
   {
      float depthAddPerIter = -1.0f / options.m_numItersPerFrame;
      float depth = options.m_useFrontToBack ? 1.0f - depthAddPerIter : 1.0f;
      float frontDepth = depth;

      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

      for (int ti = 0; ti < options.m_numItersPerFrame; ++ti)
      {
         if (options.m_useIndexArray)
         {
            color = UpdateDrawColor(options, prog, color);

            if (options.m_useVBO)
               glDrawElements(mode, indices.size(), GL_UNSIGNED_SHORT, 0);
            else
               glDrawElements(mode, indices.size(), GL_UNSIGNED_SHORT, &indices[0]);
         }
         else
         {
            for (int d = 0; d < options.m_numDrawRepeats; ++d)
            {
               color = UpdateDrawColor(options, prog, color);

               prog.SetDepth(depth);

               if (options.m_useFrontToBack)
                  depth += 0.001f;
               else
                  depth -= 0.001f;

               glDrawArrays(mode, 0, numVerts);
            }
         }

         if (options.m_useFrontToBack)
         {
            depth = frontDepth + depthAddPerIter;
            frontDepth = depth;
         }
      }

      eglSwapBuffers(eglGetCurrentDisplay(), eglGetCurrentSurface(EGL_READ));
   }

   glFinish();

   // Get end time
   int   t1 = getTimeNowMs();

   // Check for any GL errors (should not be any)
   GLenum err = glGetError();

   if (err != GL_NO_ERROR)
      std::cerr << "GL error reported: " << GLErrToString(err) << "\n";

   // Print stats
   int   numTrianglesInGrid    = options.m_numRectsInGrid * 2;
   int   numTrianglesPerFrame  = numTrianglesInGrid       * options.m_numDrawRepeats * options.m_numItersPerFrame;
   int   numTrianglesPerSample = numTrianglesPerFrame     * options.m_numSampleFrames;
   float numPixelsInGrid       = options.m_numRectsInGrid * options.m_rectWidthInPixels * options.m_rectHeightInPixels;
   float numPixelsPerFrame     = numPixelsInGrid          * options.m_numDrawRepeats * options.m_numItersPerFrame;
   float numPixelsPerSample    = numPixelsPerFrame        * options.m_numSampleFrames;

   float trianglesPerSecond = numTrianglesPerSample / ((float)(t1 - t0) / 1000.0f);
   float pixelsPerSecond    = numPixelsPerSample    / ((float)(t1 - t0) / 1000.0f);

   os << (int)trianglesPerSecond << ", " << (int)pixelsPerSecond << ", ";
   options.Dump(os);
   os << "\n";
}

void RunTextureLoadTest(const Options &options, std::ostream &os)
{
   GLFormatInfo info = GetFormatInfo(options.m_textureLoadBits);

   std::vector<GLubyte> data(options.m_textureLoadWidth * options.m_textureLoadHeight * info.m_size);
   PNGImage image;
   void * p;
   int width, height;
   float area;

   // setup defaults for width, height.  Could be overridden if the user specifies a texture to load
   width = options.m_textureLoadWidth;
   height = options.m_textureLoadHeight;
   area = (float)options.m_textureLoad * (width * height);

   // image is loaded from either the above host memory, or in the nexus case, use device memory
   // which provokes QPU texture download
   if (options.m_texFile != "")
   {
      if (!LoadPNG(options.m_texFile.c_str(), DF_8888, &image))
      {
         Error("Texture not found: ", options.m_texFile.c_str());
         p = &data[0];
      }
      else
      {
         p = image.mBytes;
         // respecify the width and height from the image
         width = image.mWidth;
         height = image.mHeight;
         os << "Image dimentions retrieved from PNG file - " << width << ", " << height << "\n";
      }
   }
   else
   {
      p = &data[0];
   }

   // Start timing
   int   t0    = getTimeNowMs();

   if (options.m_textureLoadBits != TEX_FMT_ETC1)
   {
      // recalculate the area if we are not compressed -> it could have changed in the image load
      area = (float)options.m_textureLoad * (width * height);
      for (unsigned int i = 0; i < options.m_textureLoad; ++i)
         glTexImage2D(GL_TEXTURE_2D, 0, info.m_internalFormat, width, height, 0, info.m_format, info.m_type, p);
   }
   else
   {
      // doesn't make any sense to pass a loaded PNG to the compressed texture loader
      for (unsigned int i = 0; i < options.m_textureLoad; ++i)
         glCompressedTexImage2D(GL_TEXTURE_2D, 0, info.m_internalFormat, options.m_textureLoadWidth, options.m_textureLoadHeight, 0,
                                options.m_textureLoadWidth * options.m_textureLoadHeight / 2, &data[0]);
   }

   GLenum   err = glGetError();

   if (err != GL_NO_ERROR)
      os << "GL error\n";

   // Stop timing
   int   t1 = getTimeNowMs();

   if (t1 == t0)
   {
      os << "Zero elapsed time, cannot calculate speed";
   }
   else
   {
      float time = (float)(t1 - t0) / 1000.0f;
      float mTexPerSecond = (area / 1000000.f) / time;
      os << mTexPerSecond << ", ";
      options.Dump(os);
   }

   if (options.m_texFile != "")
   {
      FreePNG(&image);
   }

   os << "\n";
}

void RunTextureFilterTest(const Options &options, std::ostream &os)
{
   float    factorW = 2.0f / options.m_vpW;
   float    factorH = 2.0f / options.m_vpH;

   float    x0 = -1.0f;
   float    y0 = -1.0f;
   float    x1 = x0 + factorW * options.m_rectWidthInPixels;
   float    y1 = y0 + factorH * options.m_rectHeightInPixels;

   float vertexData[] =
   {
      // Pos, tc
      x0, y0, 0.0f, 0.0f,
      x0, y1, 0.0f, 1.0f,
      x1, y0, 1.0f, 0.0f,
      x1, y1, 1.0f, 1.0f
   };

   if (options.m_texFile != "")
   {
      std::string file      = options.m_texFile;
      TexFmt_t    format    = options.m_textureFilterBits;
      GLenum      magFilter = options.MagFilterMode();
      GLenum      minFilter = options.MinFilterMode();
      GLuint      levels    = options.m_textureFilterLvls;

      if (options.m_textureFilterBits != TEX_FMT_ETC1)
         LoadFilterTexture(file.c_str(), format, magFilter, minFilter, levels);
      else
         LoadFilterTexturePacked(file.c_str(), format, magFilter, minFilter, levels);
   }

   Options  opt(options);

   opt.m_useTransform    = false;
   opt.m_useVaryingColor = false;
   opt.m_useVaryingDepth = false;
   Program  prog(opt);

   GLBuffer   vertexBuffer(GL_ARRAY_BUFFER);

   vertexBuffer.Bind();
   vertexBuffer.Data(sizeof(vertexData), &vertexData[0]);

   prog.SetPosition(2, GL_FLOAT, 4 * sizeof(float), (void *)(0 * sizeof(float)));
   prog.SetTexCoord(2, GL_FLOAT, 4 * sizeof(float), (void *)(2 * sizeof(float)));
   prog.SetTex(0);

   // Make sure state is what we think it should be
   glDisable(GL_DEPTH_TEST);
   glDisable(GL_CULL_FACE);
   glDisable(GL_STENCIL_TEST);

   if (opt.m_useBlend)
   {
      glEnable(GL_BLEND);
      glBlendFunc(GL_ONE_MINUS_SRC_COLOR, GL_CONSTANT_COLOR);
      glBlendColor(0.5f, 0.7f, 0.5f, 1.0f);
   }
   else
      glDisable(GL_BLEND);

   glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
   glClearDepthf(1.0f);

   // Start timing
   int   t0    = getTimeNowMs();

   for (int f = 0; f < options.m_numSampleFrames; ++f)
   {
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      for (unsigned int i = 0; i < options.m_textureFilter; ++i)
         glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

      eglSwapBuffers(eglGetCurrentDisplay(), eglGetCurrentSurface(EGL_READ));
   }

   // Stop timing
   int   t1 = getTimeNowMs();

   GLenum   err = glGetError();

   if (err != GL_NO_ERROR)
      os << "GL error\n";

   float numPixels       = options.m_numSampleFrames * options.m_textureFilter * options.m_rectWidthInPixels * options.m_rectHeightInPixels;
   float pixelsPerSecond = numPixels / ((float)(t1 - t0) / 1000.0f);

   os << (int)pixelsPerSecond << ", ";
   options.Dump(os);
   os << "\n";

   glDisable(GL_BLEND);
}

void RunTest(const Options &options, std::ostream &os)
{
   static Mode_t lastMode = MODE_NONE;

   // If we are changing mode, output a header
   if (lastMode != options.m_mode)
   {
      lastMode = options.m_mode;

      switch (options.m_mode)
      {
      case MODE_NORMAL:
         os << "Triangles/sec, Pixels/sec, ";
         break;

      case MODE_TEXTURE_LOAD:
         os << "Texture loads, Mpix/sec, ";
         break;

      case MODE_TEXTURE_FILTER:
         os << "Pixels/sec, ";
         break;

      case MODE_NONE:
      default:
         os << "Error: unknown mode";
         break;
      }

      options.DumpHeader(os);
      os << "\n";
   }


   switch (options.m_mode)
   {
   case MODE_NORMAL:
      RunRenderTest(options, os);
      break;
   case MODE_TEXTURE_LOAD:
      RunTextureLoadTest(options, os);
      break;
   case MODE_TEXTURE_FILTER:
      RunTextureFilterTest(options, os);
      break;
   case MODE_NONE:
   default:
      os << "Error: unknown mode";
      break;
   }
}

///////////////////////////////////////////////////////////////////////////////
// RunTest
//
// Runs a set of tests specified (1 per line) in the input stream
// Logs results in the output stream
///////////////////////////////////////////////////////////////////////////////
void RunTests(const Options &defaultOptions, std::istream &is, std::ostream &os)
{
   bool  commentNewLine = true;

   do
   {
      std::string line;
      std::getline(is, line);

      if (line.size() > 0)
      {
         if (line[0] != '#')
         {
            Options  options(defaultOptions);
            options.ParseLine(line);
            RunTest(options, os);
            commentNewLine = true;
         }
         else
         {
            if (commentNewLine)
            {
               os << line << "\n";
               commentNewLine = false;
            }
         }
      }

   } while (!is.eof());
}

///////////////////////////////////////////////////////////////////////////////
// Display
//
// Runs the test on the command-line, or the set of tests in a
// batch file.
///////////////////////////////////////////////////////////////////////////////
void Display(void)
{
   std::ofstream  fs;

   if (g_options.m_outFile != "")
   {
      fs.open(g_options.m_outFile.c_str());
      if (!fs) Error("Cannot open output file ", g_options.m_outFile.c_str());
   }

   std::ostream   &os = *(g_options.m_outFile == "" ? &std::cout : &fs);

   if (g_options.m_showVersion)
   {
      GLint MAX_TEXTURE_SIZE, MAX_CUBE_MAP_TEXTURE_SIZE;
      GLint MAX_RENDERBUFFER_SIZE, MAX_VERTEX_TEXTURE_IMAGE_UNITS;
      GLint MAX_TEXTURE_IMAGE_UNITS, MAX_COMBINED_TEXTURE_IMAGE_UNITS;
      GLint MAX_VERTEX_UNIFORM_VECTORS, MAX_FRAGMENT_UNIFORM_VECTORS;
      GLint MAX_VERTEX_ATTRIBS, MAX_VARYING_VECTORS;
      GLint MAX_VIEWPORT_DIMS[2];
      glGetIntegerv(GL_MAX_TEXTURE_SIZE, &MAX_TEXTURE_SIZE);
      glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE, &MAX_CUBE_MAP_TEXTURE_SIZE);
      glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &MAX_RENDERBUFFER_SIZE);
      glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &MAX_VERTEX_TEXTURE_IMAGE_UNITS);
      glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &MAX_TEXTURE_IMAGE_UNITS);
      glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &MAX_COMBINED_TEXTURE_IMAGE_UNITS);
      glGetIntegerv(GL_MAX_VERTEX_UNIFORM_VECTORS, &MAX_VERTEX_UNIFORM_VECTORS);
      glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_VECTORS, &MAX_FRAGMENT_UNIFORM_VECTORS);
      glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &MAX_VERTEX_ATTRIBS);
      glGetIntegerv(GL_MAX_VARYING_VECTORS, &MAX_VARYING_VECTORS);
      glGetIntegerv(GL_MAX_VIEWPORT_DIMS, MAX_VIEWPORT_DIMS);
      os << "Vendor,                           " << glGetString(GL_VENDOR) << "\n";
      os << "Version,                          " << glGetString(GL_VERSION) << "\n";
      os << "Renderer,                         " << glGetString(GL_RENDERER) << "\n";
      os << "SL version,                       " << glGetString(GL_SHADING_LANGUAGE_VERSION) << "\n";
      os << "MAX_TEXTURE_SIZE,                 " << MAX_TEXTURE_SIZE << "\n";
      os << "MAX_CUBE_MAP_TEXTURE_SIZE,        " << MAX_CUBE_MAP_TEXTURE_SIZE << "\n";
      os << "MAX_RENDERBUFFER_SIZE,            " << MAX_RENDERBUFFER_SIZE << "\n";
      os << "MAX_VERTEX_TEXTURE_IMAGE_UNITS,   " << MAX_VERTEX_TEXTURE_IMAGE_UNITS << "\n";
      os << "MAX_TEXTURE_IMAGE_UNITS,          " << MAX_TEXTURE_IMAGE_UNITS << "\n";
      os << "MAX_COMBINED_TEXTURE_IMAGE_UNITS, " << MAX_COMBINED_TEXTURE_IMAGE_UNITS << "\n";
      os << "MAX_VERTEX_UNIFORM_VECTORS,       " << MAX_VERTEX_UNIFORM_VECTORS << "\n";
      os << "MAX_FRAGMENT_UNIFORM_VECTORS,     " << MAX_FRAGMENT_UNIFORM_VECTORS << "\n";
      os << "MAX_VERTEX_ATTRIBS,               " << MAX_VERTEX_ATTRIBS << "\n";
      os << "MAX_VARYING_VECTORS,              " << MAX_VARYING_VECTORS << "\n";
      os << "MAX_VIEWPORT_DIMS,                " << MAX_VIEWPORT_DIMS[0] << ", " << MAX_VIEWPORT_DIMS[1] << "\n";
   }

   // Batch mode reads options from file
   if (g_options.UseBatchMode())
   {
      std::ifstream  is(g_options.m_batchFile.c_str());

      if (!is)
         Error("Cannot open batch file ", g_options.m_batchFile.c_str());

      // Run from file, g_options provides default settings
      RunTests(g_options, is, os);
   }
   else
   {
      // Just use command-line options
      RunTest(g_options, os);
   }
}

///////////////////////////////////////////////////////////////////////////////
// ArgMatch, ParseArg, ParseGlobalArgOptions
//
// Argument parsing for options
///////////////////////////////////////////////////////////////////////////////
bool ArgMatch(const char *arg, const char *patt)
{
   return strncmp(arg, patt, strlen(patt)) == 0;
}

unsigned int ParseArg(const char *arg, Options *options)
{
   if (ArgMatch(arg, "+m"))
   {
      options->m_useMultisample = true;
      return 1;
   }
   if (ArgMatch(arg, "+p"))
   {
      options->m_usePreservingSwap = true;
      return 1;
   }
   if (ArgMatch(arg, "+s"))
   {
      options->m_stretchToFit = true;
      return 1;
   }
   if (ArgMatch(arg, "d="))
   {
      if (sscanf(arg, "d=%ux%u", (unsigned int *)&options->m_vpW, (unsigned int *)&options->m_vpH) == 2)
         return 1;
   }
   if (ArgMatch(arg, "bpp="))
   {
      if (sscanf(arg, "bpp=%u", (unsigned int *)&options->m_bpp) == 1)
         return 1;
   }
   if (ArgMatch(arg, "-depth"))
   {
      options->m_useDepth = false;
      return 1;
   }
   if (ArgMatch(arg, "-stencil"))
   {
      options->m_useStencil = false;
      return 1;
   }
   if (ArgMatch(arg, "textureLoad="))
   {
      options->m_mode = MODE_TEXTURE_LOAD;
      if (sscanf(arg, "textureLoad=%u", &options->m_textureLoad) == 1)
         return 1;
   }
   if (ArgMatch(arg, "textureLoadSize="))
   {
      options->m_mode = MODE_TEXTURE_LOAD;
      if (sscanf(arg, "textureLoadSize=%ux%u", &options->m_textureLoadWidth, &options->m_textureLoadHeight) == 2)
         return 1;
   }
   if (ArgMatch(arg, "textureLoadBits="))
   {
      options->m_mode = MODE_TEXTURE_LOAD;
      if (strcmp(arg, "textureLoadBits=rgb888") == 0)
      {
         options->m_textureLoadBits = TEX_FMT_RGB888;
         return 1;
      }
      if (strcmp(arg, "textureLoadBits=rgba8888") == 0)
      {
         options->m_textureLoadBits = TEX_FMT_RGBA8888;
         return 1;
      }
      if (strcmp(arg, "textureLoadBits=rgb565") == 0)
      {
         options->m_textureLoadBits = TEX_FMT_RGB565;
         return 1;
      }
      if (strcmp(arg, "textureLoadBits=etc1") == 0)
      {
         options->m_textureLoadBits = TEX_FMT_ETC1;
         return 1;
      }
      if (strcmp(arg, "textureLoadBits=la88") == 0)
      {
         options->m_textureLoadBits = TEX_FMT_LA88;
         return 1;
      }
      if (strcmp(arg, "textureLoadBits=a8") == 0)
      {
         options->m_textureLoadBits = TEX_FMT_A8;
         return 1;
      }
      if (strcmp(arg, "textureLoadBits=l8") == 0)
      {
         options->m_textureLoadBits = TEX_FMT_L8;
         return 1;
      }
   }
   else if (ArgMatch(arg, "textureFilterMode="))
   {
      options->m_mode = MODE_TEXTURE_FILTER;
      if (strcmp(arg, "textureFilterMode=nearest") == 0)
      {
         options->m_textureFilterMode = GL_NEAREST;
         return 1;
      }
      if (strcmp(arg, "textureFilterMode=linear") == 0)
      {
         options->m_textureFilterMode = GL_LINEAR;
         return 1;
      }
      if (strcmp(arg, "textureFilterMode=linear+mipmap") == 0)
      {
         options->m_textureFilterMode = GL_LINEAR_MIPMAP_NEAREST;
         return 1;
      }
      if (strcmp(arg, "textureFilterMode=nearest+mipmap") == 0)
      {
         options->m_textureFilterMode = GL_NEAREST_MIPMAP_NEAREST;
         return 1;
      }
      if (strcmp(arg, "textureFilterMode=trilinear") == 0)
      {
         options->m_textureFilterMode = GL_LINEAR_MIPMAP_LINEAR;
         return 1;
      }
   }
   if (ArgMatch(arg, "textureFilterLevels="))
   {
      options->m_mode = MODE_TEXTURE_FILTER;
      if (sscanf(arg, "textureFilterLevels=%d", (int *)&options->m_textureFilterLvls) == 1)
         return 1;
   }
   if (ArgMatch(arg, "textureFilterBits="))
   {
      options->m_mode = MODE_TEXTURE_FILTER;
      if (strcmp(arg, "textureFilterBits=rgb888") == 0)
      {
         options->m_textureFilterBits = TEX_FMT_RGB888;
         return 1;
      }
      if (strcmp(arg, "textureFilterBits=rgba8888") == 0)
      {
         options->m_textureFilterBits = TEX_FMT_RGBA8888;
         return 1;
      }
      if (strcmp(arg, "textureFilterBits=rgb565") == 0)
      {
         options->m_textureFilterBits = TEX_FMT_RGB565;
         return 1;
      }
      if (strcmp(arg, "textureFilterBits=etc1") == 0)
      {
         options->m_textureFilterBits = TEX_FMT_ETC1;
         return 1;
      }
   }
   else if (ArgMatch(arg, "textureFilter="))
   {
      options->m_mode = MODE_TEXTURE_FILTER;
      if (sscanf(arg, "textureFilter=%d", (int *)&options->m_textureFilter) == 1)
      {
         return 1;
      }
   }
   else if (ArgMatch(arg, "repeat="))
   {
      if (sscanf(arg, "repeat=%d", &options->m_numDrawRepeats) == 1)
         return 1;
   }
   else if (ArgMatch(arg, "iters="))
   {
      if (sscanf(arg, "iters=%d", &options->m_numItersPerFrame) == 1)
         return 1;
   }
   else if (ArgMatch(arg, "side="))
   {
      if (sscanf(arg, "side=%fx%f", &options->m_rectWidthInPixels, &options->m_rectHeightInPixels) == 2)
      {
         return 1;
      }

      if (sscanf(arg, "side=%f", &options->m_rectWidthInPixels) == 1)
      {
         options->m_rectHeightInPixels = options->m_rectWidthInPixels;
         return 1;
      }
   }
   else if (ArgMatch(arg, "rects="))
   {
      if (sscanf(arg, "rects=%d", &options->m_numRectsInGrid) == 1)
         return 1;
   }
   else if (ArgMatch(arg, "frames="))
   {
      if (sscanf(arg, "frames=%d", &options->m_numSampleFrames) == 1)
         return 1;
   }
   else if (ArgMatch(&arg[1], "vbo"))
   {
      options->m_useVBO = arg[0] != '-';
      return 1;
   }
   else if (ArgMatch(&arg[1], "transform"))
   {
      options->m_useTransform = arg[0] != '-';
      return 1;
   }
   else if (ArgMatch(&arg[1], "index"))
   {
      options->m_useIndexArray = arg[0] != '-';
      return 1;
   }
   else if (ArgMatch(&arg[1], "depth") || ArgMatch(&arg[1], "ztest"))
   {
      options->m_useDepthTest = arg[0] != '-';
      return 1;
   }
   else if (ArgMatch(&arg[1], "varyColor"))
   {
      options->m_useVaryingColor = arg[0] != '-';
      return 1;
   }
   else if (ArgMatch(&arg[1], "varyDepth"))
   {
      options->m_useVaryingDepth = arg[0] != '-';
      return 1;
   }
   else if (ArgMatch(&arg[1], "ftob"))
   {
      options->m_useFrontToBack = arg[0] != '-';
      return 1;
   }
   else if (ArgMatch(&arg[1], "strip"))
   {
      options->m_useStrip = arg[0] != '-';
      return 1;
   }
   else if (ArgMatch(&arg[1], "blend"))
   {
      options->m_useBlend = arg[0] != '-';
      return 1;
   }
   else if (ArgMatch(&arg[1], "version"))
   {
      options->m_showVersion = arg[0] != '-';
      return 1;
   }
   else if (ArgMatch(arg, "elem="))
   {
      if (sscanf(arg, "elem=%d", &options->m_positionSize) == 1)
      {
         if (options->m_positionSize >= 2 && options->m_positionSize <= 4)
            return 1;
      }
   }
   else if (ArgMatch(arg, "tex="))
   {
      std::vector<char> file(strlen(arg));

      sscanf(arg, "tex=%s", &file[0]);
      options->m_texFile = &file[0];

      return 1;
   }
   else if (ArgMatch(arg, "batch="))
   {
      std::vector<char> file(strlen(arg));

      sscanf(arg, "batch=%s", &file[0]);
      options->m_batchFile = &file[0];

      return 1;
   }
   else if (ArgMatch(arg, "out="))
   {
      std::vector<char> file(strlen(arg));

      sscanf(arg, "out=%s", &file[0]);
      options->m_outFile = &file[0];

      return 1;
   }
   else if (ArgMatch(arg, "client="))
   {
      std::vector<char> file(strlen(arg));

      if (sscanf(arg, "client=%u", &options->m_clientId))
         return 1;
   }

   return 0;
}


static void BppToChannels(int bpp, int *r, int *g, int *b, int *a)
{
   switch (bpp)
   {
   default:
   case 16:             /* 16-bit RGB (565)  */
      *r = 5;
      *g = 6;
      *b = 5;
      *a = 0;
      break;

   case 32:             /* 32-bit RGBA       */
      *r = 8;
      *g = 8;
      *b = 8;
      *a = 8;
      break;

   case 24:             /* 24-bit RGB        */
      *r = 8;
      *g = 8;
      *b = 8;
      *a = 0;
      break;
   }
}

bool InitEGL(NativeWindowType egl_win)
{
   EGLDisplay egl_display      = 0;
   EGLSurface egl_surface      = 0;
   EGLContext egl_context      = 0;
   EGLConfig *egl_config;
   EGLint     major_version;
   EGLint     minor_version;
   int        config_select    = 0;
   int        configs;

   /*
      Specifies the required configuration attributes.
      An EGL "configuration" describes the pixel format and type of
      surfaces that can be used for drawing.
      For now we just want to use a 16 bit RGB surface that is a
      Window surface, i.e. it will be visible on screen. The list
      has to contain key/value pairs, terminated with EGL_NONE.
   */
   int   want_red   = 0;
   int   want_green = 0;
   int   want_blue  = 0;
   int   want_alpha = 0;

   BppToChannels(g_options.m_bpp, &want_red, &want_green, &want_blue, &want_alpha);

   /*
      Step 1 - Get the EGL display.
   */
   egl_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
   if (egl_display == EGL_NO_DISPLAY)
   {
      printf("eglGetDisplay() failed, did you register any exclusive displays\n");
      return false;
   }

   /*
      Step 2 - Initialize EGL.
      EGL has to be initialized with the display obtained in the
      previous step. We cannot use other EGL functions except
      eglGetDisplay and eglGetError before eglInitialize has been
      called.
   */
   if (!eglInitialize(egl_display, &major_version, &minor_version))
   {
      printf("eglInitialize() failed\n");
      return false;
   }

   /*
      Step 3 - Get the number of configurations to correctly size the array
      used in step 4
   */
   if (!eglGetConfigs(egl_display, NULL, 0, &configs))
   {
      printf("eglGetConfigs() failed\n");
      return false;
   }

   egl_config = (EGLConfig *)alloca(configs * sizeof(EGLConfig));

   /*
      Step 4 - Find a config that matches all requirements.
      eglChooseConfig provides a list of all available configurations
      that meet or exceed the requirements given as the second
      argument.
   */

   {
      const int   NUM_ATTRIBS = 25;
      EGLint      *attr = (EGLint *)malloc(NUM_ATTRIBS * sizeof(EGLint));
      int         i = 0;

      attr[i++] = EGL_RED_SIZE;        attr[i++] = want_red;
      attr[i++] = EGL_GREEN_SIZE;      attr[i++] = want_green;
      attr[i++] = EGL_BLUE_SIZE;       attr[i++] = want_blue;
      attr[i++] = EGL_ALPHA_SIZE;      attr[i++] = want_alpha;
      attr[i++] = EGL_SURFACE_TYPE;    attr[i++] = EGL_WINDOW_BIT;
      attr[i++] = EGL_RENDERABLE_TYPE; attr[i++] = EGL_OPENGL_ES2_BIT;

      attr[i++] = EGL_DEPTH_SIZE;
      if (g_options.m_useDepth)
         attr[i++] = 24;
      else
         attr[i++] = 0;

      attr[i++] = EGL_STENCIL_SIZE;
      if (g_options.m_useStencil)
         attr[i++] = 8;
      else
         attr[i++] = 0;

      if (g_options.m_useMultisample)
      {
         attr[i++] = EGL_SAMPLE_BUFFERS; attr[i++] = 1;
         attr[i++] = EGL_SAMPLES;        attr[i++] = 4;
      }

      attr[i++] = EGL_NONE;

      assert(i <= NUM_ATTRIBS);

      if (!eglChooseConfig(egl_display, attr, egl_config, configs, &configs) || (configs == 0))
      {
         printf("eglChooseConfig() failed");
         return false;
      }

      free(attr);
   }

   for (config_select = 0; config_select < configs; config_select++)
   {
      /*
         Configs with deeper color buffers get returned first by eglChooseConfig.
         Applications may find this counterintuitive, and need to perform additional processing on the list of
         configs to find one best matching their requirements. For example, specifying RGBA depths of 565
         could return a list whose first config has a depth of 888.
      */

      /* Check that config is an exact match */
      EGLint red_size, green_size, blue_size, alpha_size, depth_size;

      eglGetConfigAttrib(egl_display, egl_config[config_select], EGL_RED_SIZE,   &red_size);
      eglGetConfigAttrib(egl_display, egl_config[config_select], EGL_GREEN_SIZE, &green_size);
      eglGetConfigAttrib(egl_display, egl_config[config_select], EGL_BLUE_SIZE,  &blue_size);
      eglGetConfigAttrib(egl_display, egl_config[config_select], EGL_ALPHA_SIZE, &alpha_size);
      eglGetConfigAttrib(egl_display, egl_config[config_select], EGL_DEPTH_SIZE, &depth_size);

      if ((red_size == want_red) && (green_size == want_green) && (blue_size == want_blue) && (alpha_size == want_alpha))
      {
         printf("Selected config: R=%d G=%d B=%d A=%d Depth=%d\n", red_size, green_size, blue_size, alpha_size, depth_size);
         break;
      }
   }

   if (config_select == configs)
   {
      printf("No suitable configs found\n");
      return false;
   }

   /*
      Step 5 - Create a surface to draw to.
      Use the config picked in the previous step and the native window
      handle to create a window surface.
   */
   egl_surface = eglCreateWindowSurface(egl_display, egl_config[config_select], egl_win, NULL);
   if (egl_surface == EGL_NO_SURFACE)
   {
      eglGetError(); /* Clear error */
      egl_surface = eglCreateWindowSurface(egl_display, egl_config[config_select], NULL, NULL);
   }

   if (egl_surface == EGL_NO_SURFACE)
   {
      printf("eglCreateWindowSurface() failed\n");
      return false;
   }

   /* Only use preserved swap if you need the contents of the frame buffer to be preserved from
    * one frame to the next
    */
   if (g_options.m_usePreservingSwap)
   {
      printf("Using preserved swap.  Application will run slowly.\n");
      eglSurfaceAttrib(egl_display, egl_surface, EGL_SWAP_BEHAVIOR, EGL_BUFFER_PRESERVED);
   }

   /*
      Step 6 - Create a context.
      EGL has to create a context for OpenGL ES. Our OpenGL ES resources
      like textures will only be valid inside this context (or shared contexts)
   */
   {
      EGLint     ctx_attrib_list[3] =
      {
           EGL_CONTEXT_CLIENT_VERSION, 2, /* For ES2 */
           EGL_NONE
      };

      egl_context = eglCreateContext(egl_display, egl_config[config_select], EGL_NO_CONTEXT, ctx_attrib_list);
      if (egl_context == EGL_NO_CONTEXT)
      {
         printf("eglCreateContext() failed");
         return false;
      }
   }

   /*
      Step 7 - Bind the context to the current thread and use our
      window surface for drawing and reading.
      We need to specify a surface that will be the target of all
      subsequent drawing operations, and one that will be the source
      of read operations. They can be the same surface.
   */
   eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_context);

   eglSwapInterval(egl_display, 0);

   return true;
}


bool InitDisplay(float *aspect)
{
   NXPL_NativeWindowInfoEXT   win_info;

   eInitResult res = InitPlatformAndDefaultDisplay(&nexus_display, aspect, g_options.m_vpW, g_options.m_vpH, false);
   if (res != eInitSuccess)
      return false;

   /* Register with the platform layer */
   NXPL_RegisterNexusDisplayPlatform(&nxpl_handle, nexus_display);

   NXPL_GetDefaultNativeWindowInfoEXT(&win_info);

   win_info.x = 0;
   win_info.y = 0;
   win_info.width = g_options.m_vpW;
   win_info.height = g_options.m_vpH;
   win_info.stretch = g_options.m_stretchToFit;
   win_info.clientID = g_options.m_clientId;

   native_window = NXPL_CreateNativeWindowEXT(&win_info);

   /* Initialise EGL now we have a 'window' */
   if (!InitEGL(native_window))
      return false;

   return true;
}



int main(int argc, char** argv)
{
   // Message to print to show what args are accepted
   const char  *usage =
      "Usage: %s [+m] [+p] [+s] [d=WxH] [bpp=16/24/32]\n"
      "repeat=R       draw the grid R times per frame\n"
      "iters=I        perform I iterations of the per frame (grid is drawn R x I times)\n"
      "side=WxH       each rectangle in the grid is WxH pixels\n"
      "side=W         each rectangle in the grid is WxW pixels\n"
      "rects=N        grid consists of N rectangles\n"
      "frames=N       sample over N frames\n"
      "+/-vbo         use (+) or not (-) a vbo (default +)\n"
      "+/-transform   use (+) or not (-) a 3D transform in vertex shader (default -)\n"
      "+/-index       use (+) or not (-) an index array (default +)\n"
      "+/-ztest       use (+) or not (-) depth test (default -)\n"
      "+/-varyColor   use (+) or not (-) varying color between draws (default -)\n"
      "+/-varyDepth   use (+) or not (-) varying depth between draws (default -)\n"
      "+/-ftob        use (+) for varyinf depth front to back (-) for back to front (default -)\n"
      "+/-strip       use (+) or not (-) triangle strip (default +)\n"
      "+/-blend       use (+) or not (-) alpha blending\n"
      "elem=N         vertex position consists of N fields (default 3)\n"
      "tex=file       load texture file.pkm from texture folder\n"
      "\n"
      "textureLoad=n       perform texture load test n times\n"
      "textureLoadSize=WxH texture load size is W by H\n"
      "textureLoadBits=X   texture load format (X = rgba8888, rgb888, rgb565, la88, l8, a8, etc1)\n"
      "\n"
      "textureFilter=n        perform texture filter test n times\n"
      "textureFilterMode=X    use filter mode of X=nearest{+mipmap}, linear{+mipmap}, trilinear\n"
      "textureFilterBits=X    texture load format (X = rgba8888, rgb888, rgb565, etc1)\n"
      "textureFilterLevels=X  number of texture levels to load\n"
      "\n"
      "batch=file     run tests from batch file\n"
      "out=file       put results in file.csv\n";

   float        panelAspect = 1.0f;
   EGLDisplay   eglDisplay;

   if (argc < 1)
   {
      const char  *progname = argc > 0 ? argv[0] : "";
      fprintf(stderr, usage, progname);
      return 0;
   }

   for (int arg = 1; arg < argc; ++arg)
   {
      if (!ParseArg(argv[arg], &g_options))
      {
         const char  *progname = argc > 0 ? argv[0] : "";
         fprintf(stderr, usage, progname);
         return 0;
      }
   }

   /* Setup the display and EGL */
   if (InitDisplay(&panelAspect))
      Display();

   /* Terminate EGL */
   eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
   eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
   eglTerminate(eglDisplay);

   NXPL_DestroyNativeWindow(native_window);
   NXPL_UnregisterNexusDisplayPlatform(nxpl_handle);

   /* Close the platform */
   TermPlatform(nexus_display);

   return 0;
}
