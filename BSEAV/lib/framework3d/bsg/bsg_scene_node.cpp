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

   auto iter = m_children.begin() + n;

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

   auto iter = m_geometries.begin() + n;

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
