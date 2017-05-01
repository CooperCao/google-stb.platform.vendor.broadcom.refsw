/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
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
      glGetShaderInfoLog(shader, size, NULL, &err[0]);

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
         glGetProgramInfoLog(program, size, NULL, &err[0]);

         BSG_THROW("Program link failed: " <<  &err[0]);
      }
      else
         BSG_THROW("Program link failed: No error given ");
   }
}

static void ShaderSource(GLuint shader, const char *src)
{
   glShaderSource(shader, 1, &src, NULL);
}


GLProgram::GLProgram() :
   m_vert(0),
   m_frag(0),
   m_tessc(0),
   m_tesse(0),
   m_geom(0),
   m_prog(0)
{
}

GLProgram::GLProgram(
   const string &vert,
   const string &frag,
   const std::vector<std::string> &defines) :
   m_vert(0),
   m_frag(0),
   m_tessc(0),
   m_tesse(0),
   m_geom(0),
   m_prog(0)
{
   SetPrograms(vert, frag, "", "", "", defines);
}

GLProgram::GLProgram(
   const string &vert,
   const string &frag,
   const string &tc,
   const string &te,
   const string &geom,
   const std::vector<std::string> &defines) :
   m_vert(0),
   m_frag(0),
   m_tessc(0),
   m_tesse(0),
   m_geom(0),
   m_prog(0)
{
   SetPrograms(vert, frag, tc, te, geom, defines);
}

void GLProgram::SetPrograms(
   const string &vert,
   const string &frag,
   const std::vector<std::string> &defines)
{
   SetPrograms(vert, frag, "", "", "", defines);
}

void GLProgram::SetPrograms(
   const string &vert,
   const string &frag,
   const string &tc,
   const string &te,
   const string &geom,
   const std::vector<std::string> &defines)
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

#ifdef BSG_USE_ES32
      if (tc.empty() != te.empty())
         BSG_THROW("Must have both tessellation control and evaluation shaders, or neither");

      if (!tc.empty())
      {
         m_tessc = glCreateShader(GL_TESS_CONTROL_SHADER);
         glAttachShader(m_prog, m_tessc);

         m_tesse = glCreateShader(GL_TESS_EVALUATION_SHADER);
         glAttachShader(m_prog, m_tesse);

         if (m_tessc == 0 || m_tesse == 0)
            BSG_THROW("Could not create shaders");
      }

      if (!geom.empty())
      {
         m_geom = glCreateShader(GL_GEOMETRY_SHADER);
         glAttachShader(m_prog, m_geom);

         if (m_geom == 0)
            BSG_THROW("Could not create shaders");
      }
#endif

      string defs;
      for (uint32_t i = 0; i < defines.size(); i++)
         defs += string("#define ") + defines[i] + "\n";

      ShaderSource(m_vert, (defs + vert).c_str());
      ShaderSource(m_frag, (defs + frag).c_str());

      CompileShader(m_vert);
      CompileShader(m_frag);

#ifdef BSG_USE_ES32
      if (!tc.empty())
      {
         ShaderSource(m_tessc, (defs + tc).c_str());
         ShaderSource(m_tesse, (defs + te).c_str());

         CompileShader(m_tessc);
         CompileShader(m_tesse);
      }

      if (!geom.empty())
      {
         ShaderSource(m_geom, (defs + geom).c_str());
         CompileShader(m_geom);
      }
#endif

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

   map<string, GLint>::iterator iter = m_uniformLocs.find(name);
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

   map<string, GLint>::iterator iter = m_attribLocs.find(name);
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
