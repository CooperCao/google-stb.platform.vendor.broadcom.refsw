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

