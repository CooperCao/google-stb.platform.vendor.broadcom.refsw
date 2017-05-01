/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
// Prevent gl functions from being mangled in this file
#define BSG_NO_NAME_MANGLING

#include "bsg_gl_intercept.h"

static bsg::GLStateMirror s_state;

namespace bsg
{

GLTextureTargetState::GLTextureTargetState()
{
   m_boundTexture = 0;
}

GLStateMirror::GLStateMirror()
{
   m_activeTextureUnit = GL_TEXTURE0;
   m_activeProgram = 0xFFFFFFFF;

   for (uint32_t i = 0; i < NUM_ATTRIBUTES; ++i)
   {
      m_vertexAttribEnableRequired[i] = false;
      m_vertexAttribEnableCurrent[i]  = false;
   }
}

GLTextureTargetState &GLStateMirror::TargetState(GLenum target)
{
   return (target == GL_TEXTURE_2D) ?
        m_textureUnits[m_activeTextureUnit - GL_TEXTURE0].m_2d :
        m_textureUnits[m_activeTextureUnit - GL_TEXTURE0].m_cube;
}

void GLStateMirrorReset()
{
   s_state.Reset();
}

}

using namespace bsg;

extern "C"
{

void intercept_glActiveTexture(GLenum texture)
{
   if (texture != s_state.m_activeTextureUnit)
   {
      s_state.m_activeTextureUnit = texture;
      glActiveTexture(texture);
   }
}

void intercept_glBindTexture(GLenum target, GLuint texture)
{
   GLTextureTargetState &tts = s_state.TargetState(target);

   // TODO : can only do this when intercept_glDeleteTextures is complete
   //if (tts.m_boundTexture != texture)
   {
      tts.m_boundTexture = texture;
      glBindTexture(target, texture);
   }
}

void intercept_glDeleteTextures(GLsizei n, const GLuint* textures)
{
   // TODO : need to check if the texture is currently bound in any unit and reset the bound to zero
   glDeleteTextures(n, textures);
}

void intercept_glUseProgram(GLuint program)
{
   if (program != s_state.m_activeProgram)
   {
      s_state.m_activeProgram = program;
      glUseProgram(program);
   }
}

void intercept_glEnableVertexAttribArray(GLuint index)
{
   s_state.m_vertexAttribEnableRequired[index] = true;
}

void intercept_glDisableVertexAttribArray(GLuint index)
{
   s_state.m_vertexAttribEnableRequired[index] = false;
}

void GLStateMirror::SetAttributeEnables()
{
   for (uint32_t i = 0; i < NUM_ATTRIBUTES; ++i)
   {
      bool  required = m_vertexAttribEnableRequired[i];

      if (required != m_vertexAttribEnableCurrent[i])
      {
         if (required)
            glEnableVertexAttribArray(i);
         else
            glDisableVertexAttribArray(i);

         m_vertexAttribEnableCurrent[i] = required;
      }
   }
}

void GLStateMirror::Reset()
{
   for (uint32_t i = 0; i < NUM_ATTRIBUTES; ++i)
   {
      s_state.m_vertexAttribEnableCurrent[i]  = false;
      s_state.m_vertexAttribEnableRequired[i] = false;
      glDisableVertexAttribArray(0);
   }

   s_state.m_activeTextureUnit = GL_TEXTURE0;
   glActiveTexture(GL_TEXTURE0);
   s_state.m_activeProgram     = 0;
   glUseProgram(0);
}

void intercept_glDrawArrays(GLenum mode, GLint first, GLsizei count)
{
   s_state.SetAttributeEnables();
   glDrawArrays(mode, first, count);
}

void intercept_glDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices)
{
   s_state.SetAttributeEnables();
   glDrawElements(mode, count, type, indices);
}

}
