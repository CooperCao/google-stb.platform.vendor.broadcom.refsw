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

#ifndef __BSG_GATHER_VISITOR_H__
#define __BSG_GATHER_VISITOR_H__

#include "bsg_common.h"
#include "bsg_visitor.h"
#include "bsg_matrix.h"
#include "bsg_bound.h"
#include "bsg_camera.h"
#include "bsg_geometry.h"
#include "bsg_surface.h"
#include "bsg_semantic_data.h"
#include "bsg_scene_node.h"

#include <vector>

namespace bsg
{

// @cond
class DrawPacket
{
public:
   DrawPacket(uint32_t indx, Geometry *geometry, const Mat4 &model, float opacity, const SceneNodeCallbackList *callback) :
      m_geometry(geometry),
      m_surface(geometry->GetSurface(indx).GetPtr()),
      m_material(geometry->GetMaterial(indx).GetPtr()),
      m_drawCallback(geometry->GetDrawCallback(indx)),
      m_opacity(opacity),
      m_autoSort(false),
      m_render(true),
      m_sortPriority(geometry->GetSortPriority(indx)),
      m_callback(callback)
   {
      m_semanticData.SetModelMatrix(model);
   }

   Geometry *GetGeometry() const     { return m_geometry;     }
   Surface  *GetSurface()  const     { return m_surface;      }
   Material *GetMaterial() const     { return m_material;     }
   int32_t   GetSortPriority() const { return m_sortPriority; }

   const Mat4           &GetModel()        const  { return m_semanticData.GetModelMatrix();  }
   const SemanticData   &GetSemanticData() const  { return m_semanticData;                   }
   const Bound          &GetBound()        const  { return m_bound;                          }
   float                GetOpacity()       const  { return m_opacity;                        }
   bool                 GetRender()        const  { return m_render;                         }

   void SetBound(const Bound &bound)              { m_bound     = bound;     }
   void SetOpacity(float o)                       { m_opacity   = o;         }
   void SetRender(bool set)                       { m_render    = set;       }

   //! Sets the semantic data, render flag and bounds up
   void SetRenderData(const Mat4 &view, const Mat4 &proj)
   {
      m_semanticData.SetViewMatrix(view);
      m_semanticData.SetProjMatrix(proj);
      m_semanticData.SetOpacity(GetOpacity());

      if (m_callback != nullptr)
      {
         m_render = m_callback->OnRenderData(m_semanticData);
         m_callback->OnModelViewMatrix(m_semanticData.GetModelViewMatrix());
      }

      if (m_render)
         m_bound = m_surface->GetBound() * m_semanticData.GetModelViewMatrix();
   }

   bool OnDraw() const
   {
      if (m_drawCallback == 0)
         return true;

      return m_drawCallback->OnDraw();
   }

   bool GetAutoSorted() const          { return m_autoSort; }
   void SetAutoSorted(const bool &val) { m_autoSort = val; }

private:
   Geometry       *m_geometry;
   Surface        *m_surface;
   Material       *m_material;
   DrawCallback   *m_drawCallback;
   float          m_opacity;
   bool           m_autoSort;
   bool           m_render;
   int32_t        m_sortPriority;

   const SceneNodeCallbackList *m_callback;

   // Filled in after construction
   SemanticData   m_semanticData;
   Bound          m_bound;
};

class CameraPacket
{
public:
   CameraPacket(Camera *camera, const Mat4 &xform) :
      m_camera(camera),
      m_view(xform)
   {}

   Camera   *m_camera;
   Mat4     m_view;
};

typedef std::vector<DrawPacket>     DrawPackets;
typedef std::vector<CameraPacket>   CameraPackets;


class GatherVisitor : public Visitor
{
public:
   GatherVisitor();

   virtual void Visit(const SceneNode &node);
   
   void Visit(const SceneNodeHandle &nodeHandle);

   DrawPackets    &GetFrontToBackPackets() { return m_ftobPackets;    }
   DrawPackets    &GetBackToFrontPackets() { return m_btofPackets;    }
   DrawPackets    &GetNoSortPackets()      { return m_noSortPackets;  }
   CameraPackets  &GetCameraPackets()      { return m_cameraPackets;  }

   bool IsEmpty() const;

private:
   bool ConstraintsSatisfied(const SceneNode &node) const;
   void VisitNode(const SceneNode &node, const Mat4 &xformIn);

private:
   static DrawPackets                      m_ftobPackets;
   static DrawPackets                      m_btofPackets;
   static DrawPackets                      m_noSortPackets;
   static CameraPackets                    m_cameraPackets;
   static std::vector<Mat4>                m_xformStack;
   static std::vector<float>               m_opacityStack;
   static std::vector<const SceneNode *>   m_parentStack;
};

// @endcond
}

#endif /* __BSG_GATHER_VISITOR_H__ */
