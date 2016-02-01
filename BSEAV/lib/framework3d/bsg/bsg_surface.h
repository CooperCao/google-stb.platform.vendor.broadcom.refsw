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
#ifndef __BSG_SURFACE_H__
#define __BSG_SURFACE_H__

#include <vector>
#include <stdint.h>

#include "bsg_common.h"
#include "bsg_gl_buffer.h"
#include "bsg_bound.h"
#include "bsg_effect_semantics.h"
#include "bsg_semantic_data.h"
#include "bsg_material.h"
#include "bsg_pass_state.h"
#include "bsg_trackers.h"

namespace bsg
{

class Surface;

// @cond
struct SurfaceTraits
{
   typedef Surface         Base;
   typedef Surface         Derived;
   typedef SurfaceTraits   BaseTraits;
};
// @endcond

//! @addtogroup handles
//! @{
typedef Handle<SurfaceTraits>    SurfaceHandle;
//! @}

typedef std::vector<SurfaceHandle>  Surfaces;

//! Utility class for specifying cull modes.  The constructors provide the useful defaults and
//! can be used to cast from e.g. a bool to the cull mode, or from GLenum of the winding.
//! Defaults are to cull if not specified, to use CCW as front face, and to cull back faces.
class CullMode
{
public:
   CullMode()                                         { Init(true, GL_CCW,           GL_BACK);  }
   CullMode(bool mode)                                { Init(mode, GL_CCW,           GL_BACK);  }
   CullMode(GLenum frontFaceWinding, GLenum cullFace) { Init(true, frontFaceWinding, cullFace); }

   bool   GetEnableCull()       const { return m_enableCull;         }
   GLenum GetFrontFaceWinding() const { return m_frontFaceWinding;   }
   GLenum GetCullFace()         const { return m_cullFace;           }

private:
   void Init(bool enableCull, GLenum frontFaceWinding, GLenum cullFace)
   {
      m_enableCull       = enableCull;
      m_frontFaceWinding = frontFaceWinding;
      m_cullFace         = cullFace;
   }

private:
   bool     m_enableCull;
   GLenum   m_frontFaceWinding;
   GLenum   m_cullFace;
};

/** @addtogroup scenegraph
@{
*/

//! A Surface contains a set of vertices, which are rendered with a single draw call -- usually as a list of triangles.
//! They typically consist of a single VBO.
class Surface : public RefCount
{
   friend class Handle<SurfaceTraits>;

public:
   virtual ~Surface()
   {
#ifdef BSG_USE_ES3
      if (m_vao != 0)
         glDeleteVertexArrays(1, &m_vao);
#endif
   }

   //! Sets the back-face culling modes for the object.
   void SetCull(const CullMode &cull)
   {
      m_glState.SetEnableCullFace(cull.GetEnableCull());
      if (cull.GetEnableCull())
      {
         // Mark these as dirty
         m_glState.SetCullFace(cull.GetCullFace());
         m_glState.SetFrontFace(cull.GetFrontFaceWinding());
      }
      else
      {
         // Mark these as clean since culling is disabled, it doesn't matter what they are set to
         m_glState.ClearCullFace();
         m_glState.ClearFrontFace();
      }
   }

   const PassState &GetGLState() const { return m_glState; }

   void SetDrawMode(GLenum mode)
   {
      m_mode         = mode;
   }

   void SetDraw(GLenum mode, GLuint numVertices, GLuint vbBytes, const void *vbData)
   {
      m_mode         = mode;
      m_numVertices  = numVertices;

      m_vertexBuffer.Data(vbBytes, vbData);
   }

   void SetDraw(GLenum mode, GLuint numVertices, GLuint vbBytes, const void *vbData, GLuint ibBytes, const void *ibData)
   {
      SetDraw(mode, numVertices, vbBytes, vbData);

      if (ibBytes > 0)
      {
         m_indexBuffer = new GLBuffer(GL_ELEMENT_ARRAY_BUFFER);
         m_indexBuffer->Data(ibBytes, ibData);
      }
   }

   void SetDrawData(GLenum mode, GLuint numVertices, GLuint vertSize, const void *vbData)
   {
      SetDraw(mode, numVertices, numVertices * vertSize, vbData);
   }

   void UpdateVertices(GLintptr offset, GLsizeiptr size, const void *vbData)
   {
      m_vertexBuffer.SubData(offset, size, vbData);
   }

   void UpdateIndices(GLintptr offset, GLsizeiptr size, const void *ibData)
   {
      m_indexBuffer->SubData(offset, size, ibData);
   }

   // For rendering
   void        AttributesBegin(uint32_t pass, Material *material) const;
   void        AttributesEnd(uint32_t pass, Material *material)   const;
   void        UniformsBegin(uint32_t pass, const SemanticData &data, Material *material) const;

   void        Draw(uint32_t pass, Material *material, const SemanticData &data) const;

   // Accessors
   GLenum      GetMode() const            { return m_mode;        }
   GLuint      GetNumVertices() const     { return m_numVertices; }
   const Bound &GetBound() const          { return m_bound;       }

   const GLVertexPointer &GetPointer(EffectSemantics::eSemantic s) const { return m_pointers[s - EffectSemantics::eVATTR_FIRST]; }

   void        SetBound(const Bound &bound)  { m_bound = bound;   }
   void        SetPointer(EffectSemantics::eSemantic s, const GLVertexPointer &p) { m_pointers[s - EffectSemantics::eVATTR_FIRST] = p; }

   void        SetViewFrustumCull(bool cull) { m_viewFrustumCull = cull; }
   bool        GetViewFrustumCull() const    { return m_viewFrustumCull; }

#ifdef BSG_USE_ES3
   // ES3 uses "draw instanced" which can be used in shaders to replicate geometry given
   // appropriate shader code.
   void        SetNumInstances(uint32_t n)   { m_numInstances = n;       }
   uint32_t    GetNumInstances() const       { return m_numInstances;    }
#endif

protected:
   Surface() :
      m_mode(GL_TRIANGLES),
      m_numVertices(0),
      m_vertexBuffer(GL_ARRAY_BUFFER),
      m_viewFrustumCull(true)
   {
      SetCull(true);

#ifdef BSG_USE_ES3
      m_vao          = 0;
      m_lastMaterial = 0;
      m_numInstances = 1;
#endif
   }

private:
   GLenum                     m_mode;
   GLuint                     m_numVertices;
   GLVertexPointer            m_pointers[EffectSemantics::eVATTR_COUNT];
   Bound                      m_bound;
   GLBuffer                   m_vertexBuffer;
   Auto<GLBuffer>             m_indexBuffer;
   PassState                  m_glState;
   bool                       m_viewFrustumCull;
#ifdef BSG_USE_ES3
   mutable GLuint             m_vao;
   mutable Material          *m_lastMaterial;
   GLuint                     m_numInstances;
#endif
};

/** @} */
}

#endif /* __BSG_SURFACE_H__ */

