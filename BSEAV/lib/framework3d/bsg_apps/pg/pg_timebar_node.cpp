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

#include "pg_timebar_node.h"
#include "pg_info.h"
#include "pg_region.h"
#include "pg.h"

#include "bsg_shape.h"

#undef min

using namespace bsg;
using namespace pg;


TimeBarNode::TimeBarNode() :
   m_root(New),
   m_font(New),
   m_textEffect(New),
   m_tickEffect(New),
   m_lineEffect(New),
   m_tickMaterial(New),
   m_lineMaterial(New)
{
   m_font->Load("DroidSans-Bold.ttf", 20.0f);
   m_textEffect->Load("frame_text.bfx");
   m_tickEffect->Load("tick.bfx");
   m_lineEffect->Load("line.bfx");
   m_tickMaterial->SetEffect(m_tickEffect);
   m_lineMaterial->SetEffect(m_lineEffect);

   TextureHandle texture(New);
   texture->SetAutoMipmap(true);
   texture->TexImage2D(Image("tick.png", Image::eL8));
   m_tickMaterial->SetTexture("u_texSampler", texture);
   m_tickMaterial->SetUniform("u_color", Metrics::GetTimeTickColour());
   m_lineMaterial->SetUniform("u_color", Metrics::GetTimeTickColour());
}

void TimeBarNode::VisibilityCull(const Region &region)
{
   std::vector<Time>::const_iterator iter;
   uint32_t                          i;

   Time start = region.GetStartTime() - Time(1, Time::eHOURS);
   Time end   = region.GetFinishTime();

   for (i = 0, iter = m_nodeTimes.begin(); iter != m_nodeTimes.end(); ++i, ++iter)
      m_root->GetChild(i)->SetVisible(start <= m_nodeTimes[i] && end >= m_nodeTimes[i]);
}

void TimeBarNode::AppendTickGeometry(SceneNodeHandle &node, const Time &/*t*/)
{
   // Make the hour tick
   Vec2  bl(Metrics::GetTimeTickSize().X() * -0.5f, 0.0f);
   Vec2  tr(Metrics::GetTimeTickSize().X() *  0.5f, Metrics::GetTimeTickSize().Y());

   SurfaceHandle surf = QuadFactory(bl, tr, 0.0f, eZ_AXIS).MakeSurface();

   GeometryHandle geom(New);
   geom->AppendSurfaceWithSortPriority(surf, m_tickMaterial, FRAME_TEXT);

   node->AppendGeometry(geom);

   // Make the half-hour tick
   Vec2  bl2(Metrics::GetTimeTickSize().X() * -0.33f + TimeToXCoord(Time(0.5f, Time::eHOURS)), Metrics::GetTimeTickSize().Y() * 0.33f);
   Vec2  tr2(Metrics::GetTimeTickSize().X() *  0.33f + TimeToXCoord(Time(0.5f, Time::eHOURS)), Metrics::GetTimeTickSize().Y());

   SurfaceHandle surf2 = QuadFactory(bl2, tr2, 0.0f, eZ_AXIS).MakeSurface();

   GeometryHandle geom2(New);
   geom2->AppendSurfaceWithSortPriority(surf2, m_tickMaterial, FRAME_TEXT);

   node->AppendGeometry(geom2);

   // Make the line
   Vec2  bl3(Metrics::GetTimeTickSize().X() * -0.5f, Metrics::GetTimeTickSize().Y() - 0.01f);
   Vec2  tr3(Metrics::GetTimeTickSize().X() * -0.5f + TimeToXCoord(Time(1, Time::eHOURS)), Metrics::GetTimeTickSize().Y() + 0.05f);

   SurfaceHandle surf3 = QuadFactory(bl3, tr3, 0.0f, eZ_AXIS).MakeSurface();

   GeometryHandle geom3(New);
   geom3->AppendSurfaceWithSortPriority(surf3, m_lineMaterial, FRAME_TEXT);

   node->AppendGeometry(geom3);
}

GeometryHandle TimeBarNode::GetTextGeometry(const Time &t)
{
   PrintHandle geom(New);

   geom->SetFont(m_font, m_textEffect);
   geom->GetUniform<Vec4>("u_textColor").Set(Metrics::GetTimeTextColour());
   geom->SetText(t.CalendarTimeString("%H:%M"), Metrics::GetTimeTextScale(), Metrics::GetTimeTextPosition());

   return geom;
}

void TimeBarNode::SetKeepRegion(const Region &region)
{
   Time t = region.GetStartTime();
   Time end = region.GetFinishTime();

   t = bsg::Time(t.CalendarYear(), t.CalendarMonth(), t.CalendarDay(), t.CalendarHour(), 0, 0);

   // Firstly remove any nodes outside our given window
   m_root->ClearChildren();
   m_nodeTimes.clear();

   // Now add new nodes
   while (t < end)
   {
      SceneNodeHandle node(New);

      float s = TimeToXCoord(t);

      m_nodeTimes.push_back(t);

      node->AppendGeometry(GetTextGeometry(t));
      AppendTickGeometry(node, t);
      node->GetTransform().SetPosition(Vec3(s, 0.0f, 0.0f));

      m_root->AppendChild(node);

      t = t + Time(1, Time::eHOURS);
   }
}
