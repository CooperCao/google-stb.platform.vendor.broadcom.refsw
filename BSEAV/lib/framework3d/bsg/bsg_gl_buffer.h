/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
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
