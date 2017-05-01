/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "bsg_exception.h"
#include "bsg_scene_node.h"
#include "bsg_visitor.h"
#include "bsg_constraint.h"

using namespace std;

namespace bsg
{

SceneNode::SceneNode() :
   m_visible(true),
   m_opacity(1.0f)
{
}

SceneNode::~SceneNode()
{
   // Don't delete children; they are owned by the library, not by us.
   // Callbacks are deleted by the list itself
}

void SceneNode::AppendChild(const SceneNodeHandle &node)
{
   m_children.push_back(node);
}

void SceneNode::ClearChildren()
{
   m_children.clear();
}

uint32_t SceneNode::NumChildren() const
{
   return m_children.size();
}

SceneNodeHandle SceneNode::GetChild(uint32_t n) const
{
   if (n >= NumChildren())
      BSG_THROW("Invalid index");

   return m_children[n];
}

SceneNodeHandle SceneNode::RemoveChild(uint32_t n)
{
   if (n >= NumChildren())
      BSG_THROW("Invalid index");

   std::vector<SceneNodeHandle>::iterator iter = m_children.begin() + n;

   SceneNodeHandle   node = *iter;

   m_children.erase(iter);

   return node;
}

SceneNodeHandle SceneNode::RemoveChild(const string &name)
{
   int32_t  found = -1;

   for (uint32_t i = 0; found < 0 && i < NumChildren(); ++i)
      if (m_children[i].GetName() == name)
         found = i;

   return RemoveChild(found);
}

bool SceneNode::RemoveChild(const SceneNode &node)
{
   for (uint32_t i = 0; i < NumChildren(); ++i)
   {
      if (m_children[i].GetPtr() == &node)
      {
         RemoveChild(i);
         return true;
      }
   }

   return false;
}

void SceneNode::Accept(Visitor &visitor) const
{
   visitor.Visit(*this);
}

const GeometryHandle &SceneNode::GetGeometry(uint32_t n) const
{
   if (n >= m_geometries.size())
      BSG_THROW("Invalid index");

   return m_geometries[n];
}

bool SceneNode::Constrain(const SceneNode *parent, const Mat4 &xform) const
{
   return m_constraint->Visit(parent, xform);
}

Mat4 SceneNode::GetConstrainedTransform() const
{
   Mat4  trans = GetTransform().GetMatrix();

   m_constraint->GetTransform(trans);

   return trans;
}

GeometryHandle SceneNode::RemoveGeometry(uint32_t n)
{
   if (n >= NumGeometries())
      BSG_THROW("Invalid index");

   std::vector<GeometryHandle>::iterator iter = m_geometries.begin() + n;

   GeometryHandle   geom = *iter;

   m_geometries.erase(iter);

   return geom;
}

GeometryHandle SceneNode::RemoveGeometry(const std::string &name)
{
   int32_t  found = -1;

   for (uint32_t i = 0; found < 0 && i < NumGeometries(); ++i)
      if (m_geometries[i].GetName() == name)
         found = i;

   if (found == -1)
      BSG_THROW(name + " not found in node");

   return RemoveGeometry(found);
}

}
