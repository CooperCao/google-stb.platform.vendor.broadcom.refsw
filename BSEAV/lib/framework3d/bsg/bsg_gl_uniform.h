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
#ifndef __BSG_GL_UNIFORM_H__
#define __BSG_GL_UNIFORM_H__

#include "bsg_common.h"
#include "bsg_vector.h"
#include "bsg_matrix.h"
#include "bsg_gl_program.h"

#include <stdio.h>

#include <string>
#include <vector>

namespace bsg
{
// @cond
class GLUniformBase
{
public:
   GLUniformBase(const std::string &name) :
      m_name(name),
      m_dirty(true)
   {
   }

   virtual ~GLUniformBase() {}

   const std::string &GetName() const { return m_name; }

   virtual void SetProgram(const GLProgram &prog) = 0;
   virtual void Install(bool force) = 0;

protected:
   void MakeDirty()       { m_dirty = true;  }
   void MakeClean()       { m_dirty = false; }
   bool IsDirty()   const { return m_dirty;  }

private:
   std::string m_name;
   bool        m_dirty;
};


struct GLUFloat1Traits
{
   typedef  GLfloat  Data_t;
   typedef  GLfloat  Param_t;

   static void Func(GLint location, const GLfloat v1)   { glUniform1f(location, v1); }
};

struct GLUInt1Traits
{
   typedef  GLint   Data_t;
   typedef  GLint   Param_t;

   static void Func(GLint location, const GLint v1)     { glUniform1i(location, v1); }
};

struct GLUFloat2Traits
{
   typedef Vec2        Data_t;
   typedef const Vec2  &Param_t;

   static void  Func(GLint location, const Vec2 &v)   { glUniform2fv(location, 1, v.Ptr()); }
};

struct GLUInt2Traits
{
   typedef IVec2        Data_t;
   typedef const IVec2  &Param_t;

   static void  Func(GLint location, const IVec2 &v)     { glUniform2iv(location, 1, v.Ptr()); }
};

struct GLUFloat3Traits
{
   typedef Vec3        Data_t;
   typedef const Vec3  &Param_t;

   static void  Func(GLint location, const Vec3 &v)     { glUniform3fv(location, 1, v.Ptr()); }
};

struct GLUInt3Traits
{
   typedef IVec3        Data_t;
   typedef const IVec3  &Param_t;

   static void  Func(GLint location, const IVec3 &v)    { glUniform3iv(location, 1, v.Ptr()); }
};

struct GLUFloat4Traits
{
   typedef Vec4        Data_t;
   typedef const Vec4  &Param_t;

   static void  Func(GLint location, const Vec4 &v)   { glUniform4fv(location, 1, v.Ptr()); }
};

struct GLUInt4Traits
{
   typedef IVec4        Data_t;
   typedef const IVec4  &Param_t;

   static void  Func(GLint location, const IVec4 &v)     { glUniform4iv(location, 1, v.Ptr()); }
};

struct GLUMat2Traits
{
   typedef Mat2         Data_t;
   typedef const Mat2  &Param_t;

   static void  Func(GLint location, const Mat2 &v)     { glUniformMatrix2fv(location, 1, false, v.Ptr()); }
};

struct GLUMat3Traits
{
   typedef Mat3         Data_t;
   typedef const Mat3  &Param_t;

   static void  Func(GLint location, const Mat3 &v)     { glUniformMatrix3fv(location, 1, false, v.Ptr()); }
};

struct GLUMat4Traits
{
   typedef Mat4         Data_t;
   typedef const Mat4  &Param_t;

   static void  Func(GLint location, const Mat4 &v)     { glUniformMatrix4fv(location, 1, false, v.Ptr()); }
};

#ifdef BSG_USE_ES3
struct GLUMat2x3Traits
{
   typedef Mat2x3         Data_t;
   typedef const Mat2x3  &Param_t;

   static void  Func(GLint location, const Mat2x3 &v)     { glUniformMatrix2x3fv(location, 1, false, v.Ptr()); }
};

struct GLUMat2x4Traits
{
   typedef Mat2x4         Data_t;
   typedef const Mat2x4  &Param_t;

   static void  Func(GLint location, const Mat2x4 &v)     { glUniformMatrix2x4fv(location, 1, false, v.Ptr()); }
};

struct GLUMat3x2Traits
{
   typedef Mat3x2         Data_t;
   typedef const Mat3x2  &Param_t;

   static void  Func(GLint location, const Mat3x2 &v)     { glUniformMatrix3x2fv(location, 1, false, v.Ptr()); }
};

struct GLUMat3x4Traits
{
   typedef Mat3x4         Data_t;
   typedef const Mat3x4  &Param_t;

   static void  Func(GLint location, const Mat3x4 &v)     { glUniformMatrix3x4fv(location, 1, false, v.Ptr()); }
};

struct GLUMat4x2Traits
{
   typedef Mat4x2         Data_t;
   typedef const Mat4x2  &Param_t;

   static void  Func(GLint location, const Mat4x2 &v)     { glUniformMatrix4x2fv(location, 1, false, v.Ptr()); }
};

struct GLUMat4x3Traits
{
   typedef Mat4x3         Data_t;
   typedef const Mat4x3  &Param_t;

   static void  Func(GLint location, const Mat4x3 &v)     { glUniformMatrix4x3fv(location, 1, false, v.Ptr()); }
};
#endif

struct GLUFloatArrayTraits
{
   typedef float        Data_t;
   typedef const float  *Param_t;

   static void  Func(GLint location, GLsizei count, const float *v)   { glUniform1fv(location, count, v); }
};

struct GLUFloatVectorTraits
{
   typedef float                              Data_t;
   typedef const std::vector<float>           &Param_t;

   static void  Func(GLint location, GLsizei count, const float *v)   { glUniform1fv(location, count, v); }
};

struct GLUFloat2VectorTraits
{
   typedef Vec2                                 Data_t;
   typedef const std::vector<Vec2>              &Param_t;

   static void  Func(GLint location, GLsizei count, const Vec2 *v)
   {
      std::vector<float> data(count * 2);
      for (int32_t i = 0; i < count; i++)
      {
         data[i * 2 + 0] = v[i].X();
         data[i * 2 + 1] = v[i].Y();
      }
      glUniform2fv(location, count, &data[0]);
   }
};

struct GLUFloat2ArrayTraits
{
   typedef Vec2       Data_t;
   typedef const Vec2 *Param_t;

   static void  Func(GLint location, GLsizei count, const Vec2 *v)
   {
      std::vector<float> data(count * 2);
      for (int32_t i = 0; i < count; i++)
      {
         data[i * 2 + 0] = v[i].X();
         data[i * 2 + 1] = v[i].Y();
      }
      glUniform2fv(location, count, &data[0]);
   }
};

struct GLUFloat3VectorTraits
{
   typedef Vec3                                 Data_t;
   typedef const std::vector<Vec3>              &Param_t;

   static void  Func(GLint location, GLsizei count, const Vec3 *v)
   {
      std::vector<float> data(count * 3);
      for (int32_t i = 0; i < count; i++)
      {
         data[i * 3 + 0] = v[i].X();
         data[i * 3 + 1] = v[i].Y();
         data[i * 3 + 2] = v[i].Z();
      }
      glUniform3fv(location, count, &data[0]);
   }
};

struct GLUFloat3ArrayTraits
{
   typedef Vec3       Data_t;
   typedef const Vec3 *Param_t;

   static void  Func(GLint location, GLsizei count, const Vec3 *v)
   {
      std::vector<float> data(count * 3);
      for (int32_t i = 0; i < count; i++)
      {
         data[i * 3 + 0] = v[i].X();
         data[i * 3 + 1] = v[i].Y();
         data[i * 3 + 2] = v[i].Z();
      }
      glUniform3fv(location, count, &data[0]);
   }
};

struct GLUFloat4VectorTraits
{
   typedef Vec4                                 Data_t;
   typedef const std::vector<Vec4>              &Param_t;

   static void  Func(GLint location, GLsizei count, const Vec4 *v)
   {
      std::vector<float> data(count * 4);
      for (int32_t i = 0; i < count; i++)
      {
         data[i * 4 + 0] = v[i].X();
         data[i * 4 + 1] = v[i].Y();
         data[i * 4 + 2] = v[i].Z();
         data[i * 4 + 3] = v[i].W();
      }
      glUniform4fv(location, count, &data[0]);
   }
};

struct GLUFloat4ArrayTraits
{
   typedef Vec4        Data_t;
   typedef const Vec4 *Param_t;


   static void  Func(GLint location, GLsizei count, const Vec4 *v)
   {
      std::vector<float> data(count * 4);
      for (int32_t i = 0; i < count; i++)
      {
         data[i * 4 + 0] = v[i].X();
         data[i * 4 + 1] = v[i].Y();
         data[i * 4 + 2] = v[i].Z();
         data[i * 4 + 3] = v[i].W();
      }
      glUniform4fv(location, count, &data[0]);
   }
};

template <class Traits>
class GLUniform : public GLUniformBase
{
public:
   typedef typename Traits::Data_t    Data_t;
   typedef typename Traits::Param_t   Param_t;

   GLUniform() : GLUniformBase(""), m_location(-1), m_progAddr(NULL) {}

   GLUniform(const std::string &name) : GLUniformBase(name)
   {
      m_location = -1;
      m_progAddr = NULL;
   }

   void SetProgram(const GLProgram &prog)
   {
      if (&prog != m_progAddr)
      {
         m_location = prog.GetUniformLocation(GetName());
         m_progAddr = &prog;
      }
   }

   void Install(bool force)
   {
      if (IsDirty() || force)
      {
         Traits::Func(m_location, m_value);
         MakeClean();
      }
   }

   void SetValue(Param_t value)
   {
      if (IsDirty() || m_value != value)
         MakeDirty();

      m_value = value;
   }

   const GLUniform<Traits> &operator=(const GLUniform<Traits> &rhs)
   {
      SetValue(rhs.GetValue());
      return *this;
   }

   const Data_t &GetValue() const
   {
      return m_value;
   }

private:
   GLint       m_location;
   Data_t      m_value;
   const void  *m_progAddr;
};

template <class Traits>
class GLArrayUniform : public GLUniformBase
{
public:
   typedef typename Traits::Data_t    Data_t;
   typedef typename Traits::Param_t   Param_t;

   GLArrayUniform(const std::string &name) : GLUniformBase(name)
   {
      m_location = -1;
      m_progAddr = NULL;
      m_value = NULL;
      m_count = 0;
   }

   ~GLArrayUniform()
   {
      delete [] m_value;
   }

   void SetProgram(const GLProgram &prog)
   {
      if (&prog != m_progAddr)
      {
         m_location = prog.GetUniformLocation(GetName());
         m_progAddr = &prog;
      }
   }

   void Install(bool force)
   {
      if (IsDirty() || force)
      {
         Traits::Func(m_location, m_count, m_value);
         MakeClean();
      }
   }

   void SetValue(Param_t value, uint32_t count)
   {
      if (m_value == NULL || m_count != count)
      {
         // Resize if necessary
         m_count = count;
         delete [] m_value;
         m_value = new Data_t[count];
         memcpy(m_value, &value[0], sizeof(Data_t) * count);
         MakeDirty();
      }
      else
      {
         // Already right size -- has anything changed?
         for (uint32_t i = 0; i < count; ++i)
         {
            if (m_value[i] != value[i])
            {
               MakeDirty();
               m_value[i] = value[i];
            }
         }
      }
   }

   void SetValue(Param_t value)
   {
      SetValue(value, value.size());
   }

private:
   GLint       m_location;
   Data_t     *m_value;
   uint32_t    m_count;
   const void *m_progAddr;
};

typedef GLUniform<GLUFloat1Traits>  GLUniform1f;
typedef GLUniform<GLUFloat2Traits>  GLUniform2f;
typedef GLUniform<GLUFloat3Traits>  GLUniform3f;
typedef GLUniform<GLUFloat4Traits>  GLUniform4f;

typedef GLUniform<GLUInt1Traits>    GLUniform1i;
typedef GLUniform<GLUInt2Traits>    GLUniform2i;
typedef GLUniform<GLUInt3Traits>    GLUniform3i;
typedef GLUniform<GLUInt4Traits>    GLUniform4i;

typedef GLUniform<GLUMat2Traits>    GLUniformMat2;
typedef GLUniform<GLUMat3Traits>    GLUniformMat3;
typedef GLUniform<GLUMat4Traits>    GLUniformMat4;

#ifdef BSG_USE_ES3
typedef GLUniform<GLUMat2x3Traits>  GLUniformMat2x3;
typedef GLUniform<GLUMat2x4Traits>  GLUniformMat2x4;
typedef GLUniform<GLUMat3x2Traits>  GLUniformMat3x2;
typedef GLUniform<GLUMat3x4Traits>  GLUniformMat3x4;
typedef GLUniform<GLUMat4x2Traits>  GLUniformMat4x2;
typedef GLUniform<GLUMat4x3Traits>  GLUniformMat4x3;
#endif

typedef GLArrayUniform<GLUFloatArrayTraits>  GLUniform1fArray;
typedef GLArrayUniform<GLUFloat2ArrayTraits> GLUniform2fArray;
typedef GLArrayUniform<GLUFloat3ArrayTraits> GLUniform3fArray;
typedef GLArrayUniform<GLUFloat4ArrayTraits> GLUniform4fArray;

typedef GLArrayUniform<GLUFloatVectorTraits>  GLUniform1fVector;
typedef GLArrayUniform<GLUFloat2VectorTraits> GLUniform2fVector;
typedef GLArrayUniform<GLUFloat3VectorTraits> GLUniform3fVector;
typedef GLArrayUniform<GLUFloat4VectorTraits> GLUniform4fVector;

}
// @endcond
#endif

