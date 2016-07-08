/******************************************************************************
 *   Broadcom Proprietary and Confidential. (c)2011-2012 Broadcom.  All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
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

#ifndef __BSG_GL_BUFFER_H__
#define __BSG_GL_BUFFER_H__

#include "bsg_glapi.h"

#include <vector>
#include <stdint.h>

#include "bsg_common.h"
#include "bsg_no_copy.h"

namespace bsg
{
// @cond
class GLBuffer : public NoCopy
{
public:
   GLBuffer(GLenum target) :
      m_target(target)
   {
      glGenBuffers(1, &m_id);
   }

   GLBuffer(GLenum target, GLsizeiptr size, const void *data, GLenum usage = GL_STATIC_DRAW) :
      m_target(target)
   {
      glGenBuffers(1, &m_id);
      Data(size, data, usage);
   }

   ~GLBuffer()
   {
      glDeleteBuffers(1, &m_id);
   }

   void Bind() const
   {
      glBindBuffer(m_target, m_id);
   }

   void Data(GLsizeiptr size, const void *data, GLenum usage = GL_STATIC_DRAW) const
   {
      Bind();
      glBufferData(m_target, size, data, usage);
   }

   void SubData(GLintptr offset, GLsizeiptr size, const void *data) const
   {
      Bind();
      glBufferSubData(m_target, offset, size, data);
   }

private:
   GLenum   m_target;
   GLuint   m_id;
};

class GLVertexPointer
{
public:
   GLVertexPointer() :
      m_size(0),
      m_type(GL_FALSE),
      m_stride(0),
      m_ptr(0)
   {}

   GLVertexPointer(GLuint size, GLenum type, GLint stride, GLint offset) :
      m_size(size),
      m_type(type),
      m_stride(stride),
      m_ptr((const void *)(uintptr_t)offset)
   {}

   bool Pointer(GLuint index) const
   {
      if (m_size == 0)
         return false;

      glVertexAttribPointer(index, m_size, m_type, GL_FALSE, m_stride, m_ptr);
      return true;
   }

private:
   GLuint      m_size;
   GLenum      m_type;
   GLint       m_stride;
   const void  *m_ptr;
};
// @endcond
}

#endif

