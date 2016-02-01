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

#ifndef __BSG_SCENE_NODE_H__
#define __BSG_SCENE_NODE_H__

#include "bsg_common.h"
#include "bsg_maybe.h"
#include "bsg_camera.h"
#include "bsg_transform.h"
#include "bsg_library.h"
#include "bsg_geometry.h"
#include "bsg_constraint.h"
#include "bsg_scene_node_callback.h"

#include <vector>
#include <string>
#include <stdint.h>

namespace bsg
{

class SceneNode;

// @cond
struct SceneNodeTraits
{
   typedef SceneNode       Base;
   typedef SceneNode       Derived;
   typedef SceneNodeTraits BaseTraits;
};

// @endcond

//! @addtogroup handles
//! @{
typedef Handle<SceneNodeTraits>     SceneNodeHandle;
//! @}

typedef std::vector<SceneNodeHandle>   SceneNodes;

/** @addtogroup scenegraph Scene-Graph
A scene graph is composed of multiple scene nodes arranged in a tree structure.  See the "Broadcom Scene Graph.docx" document for
an in-depth description of the scene-graph and how to use it to arrange your 3D objects.

See bsg::SceneNode for the detailed method interface.

Note that applications will not interact with the scene nodes directly.  Rather they must create handles
and use the handles instead.  Handles look syntactically like pointers in applications.  See the \ref handles
documentation for more details of handles and libraries.

The SceneNode object represents one node in a scene graph.

A node contains:
- transform (1)
- geometry (>=0)
- camera (0 or 1)
- child scene nodes (>=0)

A scene is composed of multiple scene nodes arranged in a tree structure.
A SceneNode can either be an internal tree nodes and or a leaf nodes (i.e. a node with no further children).

Nodes without geometry can be used to position or animate everything below them.
Nodes with geometry generate drawing commands, and a node with a camera specifies a view.
All these elements can be combined, so a node could have a camera, geometry and a list of child nodes.

The scene graph is rendered by calling the bsg::Application::RenderSceneGraph() method.
@{
*/

//! Scene nodes are the building blocks of the scene-graph.
class SceneNode : public RefCount
{
   friend class Handle<SceneNodeTraits>;

public:
   virtual ~SceneNode();

   //! Returns the bsg::CameraHandle for this node, wrapped in a bsg::Maybe since it may not have one.
   const Maybe<CameraHandle> &GetCamera()       const { return m_camera;                     }
   //! Sets the camera handle for this node. Pass a null handle to clear the camera.
   void SetCamera(const CameraHandle &val)            { m_camera.Set(val);                   }

   //! @name Transform accessors
   //! @{
   const AnimatableTransform &GetTransform()    const { return m_transform;                  }
   AnimatableTransform &GetTransform()                { return m_transform;                  }
   void SetTransform(const AnimatableTransform &val)  { m_transform = val;                   }
   //! @}

   //! @name Transform setters
   //! Short cuts to set the transform
   //! @{
   void SetPosition(const Vec3 &pos)                  { m_transform.SetPosition(pos);           }
   void SetRotation(float angle, const Vec3 &axis)    { m_transform.SetRotation(angle, axis);   }
   void SetScale(const Vec3 &vec)                     { m_transform.SetScale(vec);              }
   //! @}

   //! @name Transform getters
   //! Short cuts to get the transform
   //! @{
   AnimatableVec3       &GetPosition()                { return m_transform.GetPosition();       }
   AnimatableQuaternion &GetRotation()                { return m_transform.GetRotation();       }
   AnimatableVec3       &GetScale()                   { return m_transform.GetScale();          }
   //! @}

   //! Returns true if this node is visible based on its visible flag.
   bool IsVisible()                             const { return m_visible;                    }
   //! Sets the visibility of the node. Invisible nodes and their children are excluded from rendering.
   void SetVisible(bool val)                          { m_visible = val;                     }

   //! Retrieves any bsg::SceneNodeCallback that was registered with this node.
   const SceneNodeCallbackList &GetCallback() const   { return m_callback;                   }

   //! Sets a bsg::SceneNodeCallback on this node it is now ownded by the node.
   void SetCallback(SceneNodeCallback *callback)      { m_callback.Add(callback);            }
   void RemoveCallback(SceneNodeCallback *callback)   { m_callback.Remove(callback);         }

   //! @name Opacity accessors
   //! The node opacity will only be respected by shader programs that use the u_opacity uniform.

   //! @{
   const AnimatableFloat &GetOpacity()          const { return m_opacity; }
   AnimatableFloat &GetOpacity()                      { return m_opacity; }
   void SetOpacity(float val)                         { m_opacity.Set(val); }
   //! @}

   //! @name Geometry accessors
   //! @{

   //! Append a geometry to this node's list of geometries. One node can contain many geometries - they will all share
   //! the same transform.
   void AppendGeometry(const GeometryHandle &geom)    { m_geometries.push_back(geom);        }

   //! Returns the number of geometries in this scene node.
   uint32_t NumGeometries() const                     { return m_geometries.size();          }

   //! Returns geomertry number n from the list of geometries.
   const GeometryHandle &GetGeometry(uint32_t n) const;

   //! Removes all geometries from this node.
   void ClearGeometry()                               { m_geometries.clear();                }

   //! Remove geometry by index
   GeometryHandle RemoveGeometry(uint32_t n);

   //! Remove geometry by name
   GeometryHandle RemoveGeometry(const std::string &name);

   //! Set node name (for debugging)
   void SetName(const std::string &name) { m_name = name; }

   //! @}

   // @cond
   void Accept(Visitor &visitor) const;
   // @endcond

   //! @name Child accessors
   //! @{

   //! Add a new child to this node.
   void        AppendChild(const SceneNodeHandle &node);
   //! Clears all child nodes.
   void        ClearChildren();
   //! Remove a specified child node by index
   SceneNodeHandle   RemoveChild(uint32_t n);
   //! Remove first specified child node by name (returns handle of removed node)
   SceneNodeHandle   RemoveChild(const std::string &name);

   //! Returns the number of child nodes.
   uint32_t    NumChildren() const;
   //! Returns the n'th child node.
   SceneNodeHandle GetChild(uint32_t n) const;
   //! @}

   //! Add a constraint to this node
   void              AddConstraint(const ConstraintHandle &constraint)  { m_constraint = constraint;                          }
   void              RemoveConstraint()                                 { m_constraint->Remove(*this); m_constraint.Clear();  }
   bool              HasConstraint() const                             { return !m_constraint.IsNull();                      }
   ConstraintHandle  GetConstraint() const                              { return m_constraint;                                }

   bool              Constrain(const SceneNode *parent, const Mat4 &xform) const;
   Mat4              GetConstrainedTransform() const;

   //! Used by constraint to remove links
   bool              RemoveChild(const SceneNode &node);

protected:
   SceneNode();

private:
   std::string             m_name;
   SceneNodes              m_children;
   Maybe<CameraHandle>     m_camera;
   AnimatableTransform     m_transform;
   Geometries              m_geometries;
   ConstraintHandle        m_constraint;
   SceneNodeCallbackList   m_callback;
   bool                    m_visible;
   AnimatableFloat         m_opacity;
};

//! @}
}

#endif /* __BSG_SCENE_NODE_H__ */

