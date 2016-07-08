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

#ifndef __BSG_SHAPE_H__
#define __BSG_SHAPE_H__

#include <vector>
#include <set>

#include "bsg_application.h"
#include "bsg_common.h"
#include "bsg_effect_semantics.h"
#include "bsg_vector.h"
#include "bsg_axis.h"
#include "bsg_surface.h"
#include "bsg_geometry.h"
#include "bsg_task.h"
#include "bsg_obj_reader.h"
#include "bsg_box.h"

namespace bsg
{

//! @addtogroup scenegraph
//! @{

//! The StdVert class represents a default vertex type comprising a position, normal and single
//! texture coordinate.  This type is used by the standard factory classes and is sufficient for
//! many  applications.  However, applications will need to extend the factories if they wish to
//! do e.g. multi-texturing (if using different texture coordinates for each texture)

Vec3 ChangeAxis(const Vec3 &v, eAxis axis);

class StdVertex
{
public:
   StdVertex() {}

   StdVertex(const Vec3 &pos, const Vec3 &norm, const Vec2 &tc) :
      m_pos(pos),
      m_norm(norm),
      m_tc(tc)
   {}

   StdVertex(const Vec3 &pos, const Vec3 &norm, const Vec2 &tc, eAxis axis) :
      m_pos(ChangeAxis(pos, axis)),
      m_norm(ChangeAxis(norm, axis)),
      m_tc(tc)
   {}

   StdVertex(const Vec3 &pos, const Vec3 &norm, const Vec2 &tc, const Vec3 &centre, eAxis axis) :
      m_pos(centre + ChangeAxis(pos, axis)),
      m_norm(ChangeAxis(norm, axis)),
      m_tc(tc)
   {}

   const Vec3 &GetPos()  const { return m_pos;  }
   const Vec3 &GetNorm() const { return m_norm; }
   const Vec2 &GetTC()   const { return m_tc;   }

private:
   Vec3  m_pos;
   Vec3  m_norm;
   Vec2  m_tc;
};

//! The shape-factory is an abstract base class for classes that create simple (single surface) geometric objects.
//! Derived classes should fill in the PushVertex method to submit the data for each vertex of the object.
//! The vertex data should be submitted in the order of the EffectSemantics::eSemantic enumeration.
//! There can only be a maximum of 8 attribute associated with each vertex.
//! If the object is drawn with an index list, then override PushIndices.
//! Overriding clases should also cal SetAttrSize() for each of the attributes that the geometry
//! has.
//!
//! Clients of the class can use MakeSurface() and MakeGeometry() methods to instantiate their objects.
//!
class ShapeFactory
{
public:
   ShapeFactory(GLenum drawMode, uint32_t numVertices, CullMode cullMode) :
      m_drawMode(drawMode),
      m_numVertices(numVertices),
      m_cullMode(cullMode),
      m_attrs(EffectSemantics::eVATTR_COUNT)
   {
      for (unsigned int i = 0; i < EffectSemantics::eVATTR_COUNT; ++i)
         m_attrs[i] = 0;
   }

   enum eAttr
   {
      ePOSITION ,
      eNORMAL,
      eTANGENT,
      eBINORMAL,
      eTEXCOORD1,
      eTEXCOORD2,
      eTEXCOORD3,
      eCOLOR,
      eUSER1,
      eUSER2,
      eUSER3,
      eUSER4,
      eUSER5,
      eUSER6,

      eATTR_FIRST = ePOSITION,
      eATTR_LAST  = eUSER6,
      eATTR_COUNT = eATTR_LAST - eATTR_FIRST + 1
   };

   SurfaceHandle  MakeSurface() const;
   GeometryHandle MakeGeometry(const MaterialHandle &material) const;

   void           SetNumVertices(uint32_t numVertices)   { m_numVertices = numVertices; }

   uint32_t       GetNumVertices() const { return m_numVertices;    }
   const CullMode &GetCullMode()   const { return m_cullMode;       }

   //! Override these methods for specific geometries, default is non-indexed
   virtual void      PushVertex(std::vector<float> &vertex, uint32_t i) const = 0;
   virtual void      PushIndices(std::vector<short> &/*indices*/)       const   {}

   GLenum GetDrawMode() const { return m_drawMode; }

   //! Use to enable or disable attributes.
   ShapeFactory &SetAttrSize(eAttr attribute, uint32_t size)
   {
      m_attrs[attribute] = size;
      return *this;
   }

   ShapeFactory &No(eAttr attribute)
   {
      m_attrs[attribute] = 0;
      return *this;
   }

   uint32_t GetAttrSize(eAttr attribute) const
   {
      return m_attrs[attribute];
   }

   uint32_t GetVertexSize() const
   {
      uint32_t size = 0;

      for (unsigned int i = 0; i < eATTR_COUNT; ++i)
         size += m_attrs[i];

      return size;
   }

   uint32_t GetOffset(eAttr attr) const
   {
      uint32_t offset = 0;

      for (int32_t i = 0; i < attr; ++i)
         offset += m_attrs[i];

      return offset;
   }

   bool Uses(eAttr attr) const
   {
      return m_attrs[attr] != 0;
   }

   void PushVertices(std::vector<float> &arr) const
   {
      for (uint32_t v = 0; v < m_numVertices; ++v)
         PushVertex(arr, v);
   }

   //! Convenience function for setting the vertex pointer up.  Checks that the attr is used and translates
   //! the attr to the internal enum.
   void SetPointer(SurfaceHandle &surf, eAttr attr, const GLVertexPointer &ptrInfo) const;

   static void       Push(std::vector<float> &arr, const Vec4 &v);
   static void       Push(std::vector<float> &arr, const Vec3 &v);
   static void       Push(std::vector<float> &arr, const Vec2 &v);
   static void       Push(std::vector<float> &arr, const float v);

private:
   // What is the size of each attribute?
   GLenum                  m_drawMode;
   uint32_t                m_numVertices;
   CullMode                m_cullMode;
   std::vector<uint32_t>   m_attrs;
};

//! Standard factories use the standard vertex class
class StdFactory : public ShapeFactory
{
public:
   StdFactory(GLenum drawMode, uint32_t numVertices, CullMode cullMode) :
      ShapeFactory(drawMode, numVertices, cullMode),
      m_vb(numVertices),
      m_index(0)
   {
      SetAttrSize(ePOSITION,  3);
      SetAttrSize(eNORMAL,    3);
      SetAttrSize(eTEXCOORD1, 2);
   }

   uint32_t AddVertex(const StdVertex &vert)
   {
      uint32_t ret = m_index;
      m_vb[ret] = vert;
      m_index++;

      return ret;
   }

   uint32_t GetIndex() const
   {
      return m_index;
   }

   uint32_t AddVertex(const Vec3 &pos, const Vec3 &norm, const Vec2 &tc, eAxis axis = eY_AXIS)
   {
      return AddVertex(StdVertex(pos, norm, tc, axis));
   }

   uint32_t AddVertex(const Vec3 &pos, const Vec3 &norm, const Vec2 &tc, const Vec3 &center, eAxis axis = eY_AXIS)
   {
      return AddVertex(StdVertex(pos, norm, tc, center, axis));
   }

   uint32_t AddVertex(float x, float y, float z, float nx, float ny, float nz, float u, float v, eAxis axis = eY_AXIS)
   {
      return AddVertex(StdVertex(Vec3(x, y, z), Vec3(nx, ny, nz), Vec2(u, v), axis));
   }

   uint32_t DupVertex()
   {
      return AddVertex(m_vb.back());
   }

   virtual void PushVertex(std::vector<float> &vertex, uint32_t i) const
   {
      if (Uses(ePOSITION))
         Push(vertex, m_vb[i].GetPos());

      if (Uses(eNORMAL))
         Push(vertex, m_vb[i].GetNorm());

      if (Uses(eTEXCOORD1))
         Push(vertex, m_vb[i].GetTC());
   }

protected:
   std::vector<StdVertex>  m_vb;
   uint32_t                m_index;
};

//! Creates axis aligned quads with the specified bottom left/top right coordinates.
//! The offset is along the specified axis.
class QuadFactory : public StdFactory
{
public:
   enum { NUM_VERTICES = 4 };

   QuadFactory(float w, float h, float offset, eAxis axis);
   QuadFactory(const Vec2 &bl, const Vec2 &tr, float offset, eAxis axis);

private:
   void Init(const Vec2 &bl, const Vec2 &tr, float offset, eAxis axis);
};

//! Creates spheres with the specified dimensions and number of steps.
class SphereFactory : public StdFactory
{
public:
   SphereFactory(const Vec3 &centre, float radius, uint32_t steps);
};

//! Creates cubes with the specified dimensions and number of steps.
class CuboidFactory : public StdFactory
{
public:
   enum { NUM_VERTICES = 24 }; // Each face has unique texture coords so 4 * 6

   CuboidFactory(float s);
   CuboidFactory(float w, float h, float d);

   virtual void PushIndices(std::vector<short> &indices) const;

private:
   void Init(float w, float h, float d);
};

//! Creates a disk with steps sectors.
class DiskFactory : public StdFactory
{
public:
   DiskFactory(const Vec3 &centre, float radius, uint32_t steps, eAxis axis);
};

//! Creates a ring with the specified number of steps.
class RingFactory : public StdFactory
{
public:
   RingFactory(const Vec3 &centre, float innerRadius, float outerRadius, uint32_t steps, eAxis axis);
};

//! Creates a cone with the specified number of steps.
class ConeFactory : public StdFactory
{
public:
   ConeFactory(const Vec3 &centre, float innerRadius, float outerRadius, float height, uint32_t isteps, eAxis axis);
};

//! Creates a rounded rectangle.
class RoundedRectFactory : public StdFactory
{
public:
   RoundedRectFactory(const Vec3 &centre, float width, float height, float cornerRadius, uint32_t isteps, eAxis axis);

   virtual void PushIndices(std::vector<short> &indices) const;

private:
   Vec2     PosToTC(const Vec3 &pos) const;
   uint32_t Fan(const Vec3 &centre, float theta1, float theta2);

private:
   std::vector<short>   m_ib;
   Vec3                 m_centre;
   float                m_width;
   float                m_height;
   float                m_cornerRadius;
   uint32_t             m_isteps;
   eAxis                m_axis;
};

//! Creates a panel with a fixed width border
class PanelFactory : public StdFactory
{
public:
   PanelFactory(const Vec2 &bl, const Vec2 &tr, float geomBevel, float texBevel, eAxis axis);
   PanelFactory(const Vec2 &bl, const Vec2 &tr, const Vec2 &geomBevel, float texBevel, eAxis axis);
   virtual ~PanelFactory() {};

   virtual void PushIndices(std::vector<short> &indices) const;
};


class ObjMaterialOptions;
class ObjReader;

//! The ObjFactory will read an Alias/Wavefront "obj" file and construct a piece of geometry.
class ObjBuffer
{
public:
   ObjBuffer(const ObjReader *reader)
   {
      (void)reader;
   }

private:

};

class ObjFactory
{
public:
   ObjFactory(const std::string &fileName);
   ~ObjFactory();

   //! Create all surfaces with this material
   GeometryHandle MakeGeometry(const MaterialHandle &material);

   //! Use existing materials (searched for by name) and return those used in the materials set (if non null)
   GeometryHandle MakeGeometry(std::set<MaterialHandle> *materials = 0);

   //! Either use existing materials, of if not found generate them internally using the options and return in the set (if non null)
   GeometryHandle MakeGeometry(const ObjMaterialOptions &matOptions, std::set<MaterialHandle> *materials = 0);

private:
   SurfaceHandle CreateSurfaceFromObj(const ObjReader &reader, uint32_t obj) const;

private:
   ObjReader   *m_reader;
   std::string m_path;
};

class ObjFactoryTask : public CallbackTask
{
public:
   //! Launch this task to load an obj file from fileName into geometry using material
   //! Derive from this class and provide an OnLoaded method to fill in terminal behavior.
   //! use as e.g. tasker.Submit(new MyObjFactoryTask(...));
   ObjFactoryTask(const std::string &fileName, const GeometryHandle &geometry, const MaterialHandle &material, uint32_t faceBatchSize = 0) :
      m_fileName(fileName),
      m_geometry(geometry),
      m_material(material),
      m_faceBatchSize(faceBatchSize),
      m_numVertices(0),
      m_numSurfaces(0)
   {
      if (m_faceBatchSize == 0)
         m_faceBatchSize = 100000;
   }

   //! Invoked in the worker thread
   virtual void OnThread();

   //! Invoked in the main thread when the file I/O has finished
   virtual void OnLoaded() {}

   //! Invoked in the main thread when finished
   virtual void OnFinished() = 0;

   virtual void DefaultCallback(bool finished)
   {
      if (finished)
         OnFinished();
   }

   const Box &GetBoundingBox() const { return m_boundingBox; }
   uint32_t  GetNumVertices()  const { return m_numVertices; }
   uint32_t  GetNumSurfaces()  const { return m_numSurfaces; }

private:
   void CreateSurface(bool hasN, bool hasTC, uint32_t vertexSize, uint32_t totalVertices);
   void SubmitVertexData(uint32_t floatOffset, uint32_t floatCount, float *vertArray);

private:
   std::auto_ptr<ObjReader> m_reader;
   std::string                m_fileName;
   GeometryHandle             m_geometry;
   MaterialHandle             m_material;
   SurfaceHandle              m_surface;
   uint32_t                   m_faceBatchSize;
   Box                        m_boundingBox;
   uint32_t                   m_numVertices;
   uint32_t                   m_numSurfaces;
};

//! @}

}

#endif /* __BSG_SHAPE_H__ */
