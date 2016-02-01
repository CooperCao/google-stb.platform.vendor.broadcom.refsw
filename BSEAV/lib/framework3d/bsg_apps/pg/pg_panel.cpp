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

#include "pg.h"
#include "pg_panel.h"
#include "pg_info.h"
#include "pg_region.h"

#include "bsg_shape.h"
#include "bsg_time.h"

#include <vector>

using namespace bsg;

namespace pg
{

class ColorCallback : public DrawCallback
{
public:
   ColorCallback(const Vec4 &color, MaterialHandle &mat) :
      m_color(color),
      m_uniform(mat->GetUniform<Vec4>("u_color"))
   {}

   virtual bool OnDraw()
   {
      m_uniform.Set(m_color);
      return true;
   }

private:
   Vec4              m_color;
   AnimTarget<Vec4>  &m_uniform;
};

Vec4 ColorOf(uint32_t line)
{
   return (line & 1) == 0 ? Metrics::GetPanelColourEven() : Metrics::GetPanelColourOdd();
}

PanelCache::PanelCache() :
   m_font(New),
   m_textEffect(New),
   m_glowEffect(New),
   m_glowMaterial(New),
   m_glowTexture(New),
   m_glowSequence(&theApp->AnimList())
{
   // TODO make this dynamic

   m_font->Load("DroidSans-Bold.ttf", Metrics::GetPanelTextPoints());
   m_textEffect->Load("panel_text.bfx");

   m_glowTexture->SetAutoMipmap(true);
   m_glowTexture->TexImage2D(Image("glow.png", Image::eL8));

   m_glowEffect->Load("glow.bfx");
   m_glowMaterial->SetEffect(m_glowEffect);
   m_glowMaterial->SetTexture("u_texture", m_glowTexture);
   m_glowMaterial->SetUniform("u_color", Metrics::GetHighlightColour());

   // Animate the glow
   uint32_t label = m_glowSequence.AppendLabel();

   Vec4 otherColor(1.0f, 0.64f, 0.0f, 1.0f);

   AnimBindingHermiteVec4 *anim = new AnimBindingHermiteVec4(&m_glowMaterial->GetUniform4f("u_color"));
   anim->Evaluator()->Init(Metrics::GetHighlightColour(), otherColor);

   m_glowSequence.AppendAnimation(anim, 1.0f);

   AnimBindingHermiteVec4 *anim2 = new AnimBindingHermiteVec4(&m_glowMaterial->GetUniform4f("u_color"));
   anim2->Evaluator()->Init(otherColor, Metrics::GetHighlightColour());

   m_glowSequence.AppendAnimation(anim2, 1.0f);
   m_glowSequence.AppendGoto(label);
   m_glowSequence.Start(Time::Now());

   if (Metrics::GetDisableStenciling())
      m_textEffect->GetPass(0)->State().SetEnableStencilTest(false);

   m_printOpt.SetAnchor(PrintOptions::eBOTTOM_LEFT);
   m_printOpt.UseBoundingBox(false);
}

PanelCache::~PanelCache()
{
}

#define USE_NODE_TRANSFORMS
#ifdef USE_NODE_TRANSFORMS
GeometryHandle PanelCache::GetPanelGeometry(uint32_t line, const ProgramInfo &prog)
{
   float s = TimeToXCoord(prog.GetStartTime());
   float f = TimeToXCoord(prog.GetEndTime());
   float w = f - s;

   //int32_t startMinute = (int32_t)prog.GetStartTime().Minutes();
   //int32_t endMinute   = (int32_t)prog.GetEndTime().Minutes();
   //int32_t duration    = (int32_t)(w * 10000);

   GeometryMap::iterator it = m_geometryMap.find(GeometryKey(line, w));

   if (it != m_geometryMap.end())
      return it->second;

   float xInset = Metrics::GetPanelXInset();
   float yInset = Metrics::GetPanelYInset();

   Vec2  bl(xInset, yInset);
   Vec2  tr(w - xInset, 1.0f);

   GeometryHandle geom(New);
   SurfaceHandle  surf;

   SurfaceMap::iterator surfIter = m_surfaceMap.find(w);

   if (surfIter != m_surfaceMap.end())
   {
      surf = surfIter->second;
   }
   else
   {
      surf = QuadFactory(bl, tr, 0.0f, eZ_AXIS).MakeSurface();
      m_surfaceMap[w] = surf;
   }

   MaterialHandle mat = GetPanelMaterial();

   geom->AppendSurfaceWithSortPriority(surf, mat, PANEL_GEOM_BASE + (int32_t)line, new ColorCallback(ColorOf(line), mat));

   m_geometryMap[GeometryKey(line, w)] = geom;

   return geom;
}

PrintHandle PanelCache::GetTextGeometry(uint32_t line, const ProgramInfo &prog, const TextKey &key)
{
   return GetTextGeometry(line, prog.GetStartTime(), prog.GetEndTime(), prog.GetTitle(), 0.0f, key);
}

PrintHandle PanelCache::GetTextGeometry(uint32_t line, const Time &start, const Time &finish, const std::string &title, float offset, const TextKey &key)
{
   TextMap::const_iterator  iter = m_textMap.find(key);

   PrintHandle geom;

   if (iter == m_textMap.end())
   {
      float limit = TimeToXCoord(finish - start) - 0.2f;

      ShortenFormatter  *formatter = new ShortenFormatter(limit);

      geom = PrintHandle(New);
      geom->SetFormatter(formatter);
      geom->SetFont(m_font, m_textEffect, m_printOpt);
      geom->SetUniform("u_textColor", Metrics::GetPanelTextColour());
      geom->SetText(title, Metrics::GetPanelTextScale(), Vec2(offset, 0.0f) + Metrics::GetPanelTextPosition());

      geom->SetSortPriority(PANEL_TEXT_BASE + line);

      m_textMap[key] = geom;
   }
   else
   {
      geom = (*iter).second;
   }

   return geom;
}

GeometryHandle PanelCache::GetHighlightGeometry(const Time &start, const Time &finish)
{
   float s = TimeToXCoord(start);
   float f = TimeToXCoord(finish);
   float w = f - s;

   Vec2  bl(0.0f, 0.0f);
   Vec2  tr(w,    1.1f);

   GeometryHandle geom = PanelFactory(bl, tr, 0.15f, 0.26f, eZ_AXIS).MakeGeometry(m_glowMaterial);
   geom->SetSortPriority(HIGHLIGHT);

   return geom;
}

void PanelCache::Clear()
{
   m_surfaceMap.clear();
   m_geometryMap.clear();
   m_textMap.clear();
}

#else
// NO NODE TRANSFORMS
GeometryHandle PanelCache::GetPanelGeometry(uint32_t line, const ProgramInfo &prog)
{
   GeometryHandle geom(New);
   SurfaceHandle  surf;

   float s = TimeToXCoord(prog.GetStartTime());
   float f = TimeToXCoord(prog.GetEndTime());
   float w = f - s;

   float xInset = Metrics::GetPanelXInset();
   float yInset = Metrics::GetPanelYInset();

   Vec2  bl(s + xInset, yInset);
   Vec2  tr(s + w - xInset, 1.0f);

   surf = QuadFactory(bl, tr, 0.0f, eZ_AXIS).MakeSurface();

   MaterialHandle mat = GetPanelMaterial();

   geom->AppendSurfaceWithSortPriority(surf, mat, PANEL_GEOM_BASE + (int32_t)line, new ColorCallback(ColorOf(line), mat));

   return geom;
}

GeometryHandle PanelCache::GetTextGeometry(uint32_t line, const ProgramInfo &prog)
{
   return GetTextGeometry(line, prog.GetStartTime(), prog.GetEndTime(), prog.GetTitle());
}

GeometryHandle PanelCache::GetTextGeometry(uint32_t line, const Time &start, const Time &finish, const std::string &title, float offset)
{
   float limit = TimeToXCoord(finish - start) - 0.2f;

   ShortenFormatter  *formatter = new ShortenFormatter(limit);

   PrintHandle geom(New);

   float s = TimeToXCoord(start);

   geom->SetFormatter(formatter);
   geom->SetFont(m_font, m_textEffect, m_printOpt);
   geom->GetUniform<Vec4>("u_textColor").Set(Metrics::GetPanelTextColour());
   geom->SetText(title, Metrics::GetPanelTextScale(), Vec2(offset, 0.0f) + Metrics::GetPanelTextPosition() + Vec2(s, 0.0f));

   geom->SetSortPriority(PANEL_TEXT_BASE + line);

   return geom;
}
#endif

PanelCache *PanelCache::GetPanelCache()
{
   static PanelCache *panelCache = 0;

   if (panelCache == 0)
      panelCache = new PanelCache;

   return panelCache;
}

PanelNode::PanelNode(uint32_t line, const ProgramInfo &prog) :
   m_root(New),
   m_textNode(New),
   m_prog(prog),
   m_shifted(false),
   m_line(line),
   m_textKey(prog.GetDuration(), prog.GetTitle())
{
#ifdef USE_NODE_TRANSFORMS
   float s = TimeToXCoord(prog.GetStartTime());
   m_root->GetTransform().SetPosition(Vec3(s, 0.0f, 0.0f));
#endif

   GeometryHandle panel = PanelCache::GetPanelCache()->GetPanelGeometry(m_line, prog);
   PrintHandle    text  = PanelCache::GetPanelCache()->GetTextGeometry(m_line, prog, m_textKey);

   m_hasShortened = dynamic_cast<ShortenFormatter*>(text->GetFormatter())->GetHasShortened();

   m_textSize = text->GetWidth();

   m_root->AppendGeometry(panel);

   m_textNode->AppendGeometry(text);
   m_root->AppendChild(m_textNode);
}

PanelNode::~PanelNode()
{
}

void PanelNode::AdjustText(const Region &activeRegion)
{
   if (!m_hasShortened)
   {
      if (GetStartTime() < activeRegion.GetStartTime() && GetFinishTime() > activeRegion.GetStartTime())
      {
         // Straddles active region start -- move the text
         float arStart = TimeToXCoord(activeRegion.GetStartTime());
         float pStart  = TimeToXCoord(m_prog.GetStartTime());
         float offset  = arStart - pStart;
         float pEnd    = TimeToXCoord(m_prog.GetEndTime()) - Metrics::GetPanelTextPosition().X() * 2.0f;

         if (arStart + m_textSize > pEnd)
            m_textNode->SetPosition(Vec3(pEnd - pStart - m_textSize, 0.0f, 0.0f));
         else
            m_textNode->SetPosition(Vec3(offset, 0.0f, 0.0f));

         m_shifted = true;
      }
      else
      {
         if (m_shifted)
         {
            m_textNode->SetPosition(Vec3());

            m_shifted = false;
         }
      }
   }
}

void PanelNode::HighlightSelection(Selection &selection, bool select)
{
   if (select)
   {
      if (!IsSelected())
         m_root->AppendGeometry(PanelCache::GetPanelCache()->GetHighlightGeometry(GetStartTime(), GetFinishTime()));

      selection.SetProgInfo(m_prog);
   }
   else
   {
      if (IsSelected())
         m_root->RemoveGeometry(1);
   }
}

bool PanelNode::IsSelected() const 
{
   return m_root->NumGeometries() > 1;
}

}

