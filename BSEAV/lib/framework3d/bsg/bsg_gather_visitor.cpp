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

#include "bsg_gather_visitor.h"
#include "bsg_scene_node.h"
#include "bsg_transform.h"
#include "bsg_constraint.h"
#include <stdio.h>

namespace bsg
{

DrawPackets                      GatherVisitor::m_ftobPackets;
DrawPackets                      GatherVisitor::m_btofPackets;
DrawPackets                      GatherVisitor::m_noSortPackets;
CameraPackets                    GatherVisitor::m_cameraPackets;
std::vector<Mat4>                GatherVisitor::m_xformStack;
std::vector<float>               GatherVisitor::m_opacityStack;
std::vector<const SceneNode *>   GatherVisitor::m_parentStack;

GatherVisitor::GatherVisitor()
{
   m_ftobPackets.clear();
   m_btofPackets.clear();
   m_noSortPackets.clear();
   m_cameraPackets.clear();
   m_xformStack.clear();
   m_opacityStack.clear();
   m_parentStack.clear();

   m_xformStack.push_back(Mat4());
   m_opacityStack.push_back(1.0f);
}

bool GatherVisitor::ConstraintsSatisfied(const SceneNode &node) const
{
   return node.Constrain(m_parentStack.back(), m_xformStack.back()); 
}

void GatherVisitor::VisitNode(const SceneNode &node, const Mat4 &xformIn)
{
   float    opacity = m_opacityStack.back() * node.GetOpacity();

   // Include our transform
   m_xformStack.push_back(xformIn);

   const Mat4  &xform = m_xformStack.back();

   m_opacityStack.push_back(opacity);
   m_parentStack.push_back(&node);

   // If there is a callback installed then inform it of our matrix.
   node.GetCallback().OnModelMatrix(xform);

   // Record the camera if we have one
   const Maybe<CameraHandle>  &camera = node.GetCamera();

   if (camera)
      m_cameraPackets.push_back(CameraPacket(camera.Get().GetPtr(), xform));

   // Iterate our geometries and surfaces
   for (uint32_t g = 0; g < node.NumGeometries(); g++)
   {
      const GeometryHandle &geom = node.GetGeometry(g);

      for (uint32_t s = 0; s < geom->NumSurfaces(); s++)
      {
         DrawPacket packet(s, geom.GetPtr(), xform, opacity, &node.GetCallback());

         switch (geom->GetMaterial(s)->GetEffect()->GetSortOrder())
         {
         default:
         case EffectOptions::eAUTO : 
            packet.SetAutoSorted(true);
            if (m_opacityStack.back() > 0.999f)
               m_ftobPackets.push_back(packet); 
            else
               m_btofPackets.push_back(packet);
            break;
         case EffectOptions::eNO_SORT       : m_noSortPackets.push_back(packet); break;
         case EffectOptions::eFRONT_TO_BACK : m_ftobPackets.push_back(packet); break;
         case EffectOptions::eBACK_TO_FRONT : m_btofPackets.push_back(packet); break;
         }
      }
   }

   // Deal with our children  
   for (uint32_t k = 0; k < node.NumChildren(); k++)
   {
      const SceneNodeHandle &child = node.GetChild(k);

      child->Accept(*this);
   }

   m_xformStack.pop_back();
   m_opacityStack.pop_back();
   m_parentStack.pop_back();
}

void GatherVisitor::Visit(const SceneNode &node)
{
   if (node.IsVisible() && node.GetOpacity() > 0.0f)
   {
      if (node.HasConstraint())
      {
         if (ConstraintsSatisfied(node))
            VisitNode(node, node.GetConstrainedTransform());
      }
      else
      {
         if (node.GetTransform().IsIdentity())
            VisitNode(node, m_xformStack.back());
         else
            VisitNode(node, m_xformStack.back() * node.GetTransform().GetMatrix());
      }
   }
}

void GatherVisitor::Visit(const SceneNodeHandle &nodeHandle)
{
   Visit(*nodeHandle.GetPtr());
}

bool GatherVisitor::IsEmpty() const
{
   return m_ftobPackets.size() == 0 && m_btofPackets.size() == 0 && m_noSortPackets.size() == 0;
}

}
