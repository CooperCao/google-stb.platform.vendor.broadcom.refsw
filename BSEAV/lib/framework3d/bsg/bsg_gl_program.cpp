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

#include "bsg_glapi.h"

#include "bsg_exception.h"
#include "bsg_gl_program.h"

#include <iostream>
#include <vector>

using namespace bsg;
using namespace std;

static void CompileShader(GLuint shader)
{
   glCompileShader(shader);

   GLint ok;
   glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);

   if (!ok)
   {
      GLint   size;
      glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &size);

      vector<char> err(size);
      glGetShaderInfoLog(shader, size, nullptr, &err[0]);
      
      BSG_THROW("Shader compile failed: " << &err[0]);
   }
}

static void LinkProgram(GLint program)
{
   glLinkProgram(program);

   GLint ok;

   glGetProgramiv(program, GL_LINK_STATUS, &ok);
   if (!ok)
   {
      GLint size;
      glGetProgramiv(program, GL_INFO_LOG_LENGTH, &size);

      if (size > 0)
      {
         vector<char> err(size);
         glGetProgramInfoLog(program, size, nullptr, &err[0]);

         BSG_THROW("Program link failed: " <<  &err[0]);
      }
      else
         BSG_THROW("Program link failed: No error given ");
   }
}

static void ShaderSource(GLuint shader, const char *src)
{
   glShaderSource(shader, 1, &src, nullptr);
}


GLProgram::GLProgram() :
   m_vert(0),
   m_frag(0),
   m_prog(0)
{
}

GLProgram::GLProgram(const string &vert, const string &frag, const std::vector<std::string> &defines) :
   m_vert(0),
   m_frag(0),
   m_prog(0)
{
   SetPrograms(vert, frag, defines);
}

void GLProgram::SetPrograms(const string &vert, const string &frag, const std::vector<std::string> &defines)
{
   try
   {
      m_prog = glCreateProgram();
      if (m_prog == 0)
         BSG_THROW("Could not create program");

      m_vert = glCreateShader(GL_VERTEX_SHADER);
      glAttachShader(m_prog, m_vert);

      m_frag = glCreateShader(GL_FRAGMENT_SHADER);
      glAttachShader(m_prog, m_frag);

      if (m_vert == 0 || m_frag == 0)
         BSG_THROW("Could not create shaders");

      string defs;
      for (uint32_t i = 0; i < defines.size(); i++)
         defs += string("#define ") + defines[i] + "\n";

      ShaderSource(m_vert, (defs + vert).c_str());
      ShaderSource(m_frag, (defs + frag).c_str());

      CompileShader(m_vert);
      CompileShader(m_frag);

      LinkProgram(m_prog);
   }
   catch (...)
   {
      Finish();
      throw;
   }
}

GLProgram::~GLProgram() 
{
   Finish();
}

void GLProgram::Finish()
{
   m_uniformLocs.clear();
   m_attribLocs.clear();

   if (m_prog != 0)
   {
      glDeleteProgram(m_prog);
      m_prog = 0;
   }

   if (m_vert != 0)
   {
      glDeleteShader(m_vert);
      m_vert = 0;
   }

   if (m_frag != 0)
   {
      glDeleteShader(m_frag);
      m_frag = 0;
   }
}

void GLProgram::Use()
{
   glUseProgram(m_prog);
}

GLint GLProgram::GetUniformLocation(const string &name) const
{
   GLint loc = -1;

   auto iter = m_uniformLocs.find(name);
   if (iter != m_uniformLocs.end())
   {
      loc = iter->second;
   }
   else
   {
      loc = glGetUniformLocation(m_prog, name.c_str());
      m_uniformLocs.insert(pair<string, GLint>(name, loc));
   }

   return loc;
}

GLint GLProgram::GetAttribLocation(const string &name) const
{
   GLint loc = -1;

   auto iter = m_attribLocs.find(name);
   if (iter != m_attribLocs.end())
   {
      loc = iter->second;
   }
   else
   {
      loc = glGetAttribLocation(m_prog, name.c_str());
      m_attribLocs.insert(pair<string, GLint>(name, loc));
   }

   return loc;
}

