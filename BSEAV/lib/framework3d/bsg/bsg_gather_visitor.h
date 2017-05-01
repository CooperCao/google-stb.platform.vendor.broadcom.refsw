/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
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

      if (m_callback != NULL)
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
