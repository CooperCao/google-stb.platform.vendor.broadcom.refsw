/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "pg_controller.h"
#include "pg_info.h"
#include "pg_region.h"
#include "pg.h"

#include "bsg_shape.h"

#include <limits.h>
#include <algorithm>

using namespace bsg;
using namespace pg;

//! Builds the scene graph and internal data structures from the programme guide data
void Controller::Initialise(const GridInfo &info, const Region &activeRegion, const Region &keepRegion)
{
   m_info = &info;

   Clean();

   m_channelRoot.resize(info.GetNumChannels());

   // Make the stenciling rectangles

   EffectHandle   stencilEffect(New);
   stencilEffect->Load("stencil_rect.bfx");
   stencilEffect->GetPass(0)->State().SetStencilFunc(GL_ALWAYS, 1, 255);

   MaterialHandle stencilMaterial(New);
   stencilMaterial->SetEffect(stencilEffect);

   EffectHandle   stencilFrameEffect(New);
   stencilFrameEffect->Load("stencil_rect.bfx");
   stencilFrameEffect->GetPass(0)->State().SetStencilFunc(GL_ALWAYS, 2, 255);

   MaterialHandle stencilFrameMaterial(New);
   stencilFrameMaterial->SetEffect(stencilFrameEffect);

   EffectHandle   pgBackEffect(New);
   pgBackEffect->Load("pg_back.bfx");

   TextureHandle pgBackTex(New);
   pgBackTex->SetAutoMipmap(true);
   pgBackTex->TexImage2D(Image("bcm_logo", "png", Image::eRGB888));

   MaterialHandle pgBackMaterial(New, "PgBack");
   pgBackMaterial->SetEffect(pgBackEffect);

   pgBackMaterial->SetTexture("u_tex", pgBackTex);

   float halfDuration = TimeToXCoord(activeRegion.GetDuration() * 0.5f);
   float halfChannels = ChannelStride() * (float)activeRegion.GetNumChannels() * 0.5f;

   /* float halfChannelFuzz = halfChannels + 0.1f; */

   GeometryHandle stencilRect = QuadFactory(Vec2(-halfDuration, -halfChannels),
                                            Vec2( halfDuration,  halfChannels),
                                            0.0f, eZ_AXIS).MakeGeometry(stencilMaterial);

   GeometryHandle pgBackRec = QuadFactory(Vec2( -halfDuration - m_channelLabelLength,  -halfChannels),
                                          Vec2(halfDuration, halfChannels),
                                            -1.0f, eZ_AXIS).MakeGeometry(pgBackMaterial);

   GeometryHandle stencilTimeRect = QuadFactory(Vec2(-halfDuration/* - Metrics::GetTimeTickSize().X() * 0.5f*/, halfChannels),
                                                Vec2( halfDuration, halfChannels + ChannelStride() * 1.0f),
                                                0.0f, eZ_AXIS).MakeGeometry(stencilFrameMaterial);

   // A tiny rect for the end tick (excluding the text above)
   GeometryHandle stencilTimeRect2 = QuadFactory(Vec2(halfDuration, halfChannels),
                                                 Vec2(halfDuration + Metrics::GetTimeTickSize().X() * 0.5f,
                                                 halfChannels + Metrics::GetTimeTickSize().Y() + 0.06f),
                                                 0.0f, eZ_AXIS).MakeGeometry(stencilFrameMaterial);

   GeometryHandle stencilChanRect = QuadFactory(Vec2(-halfDuration - m_channelLabelLength, -halfChannels),
                                                Vec2(-halfDuration, halfChannels),
                                                0.0f, eZ_AXIS).MakeGeometry(stencilFrameMaterial);

   stencilRect->SetSortPriority(PANEL_STENCIL);
   stencilTimeRect->SetSortPriority(FRAME_STENCIL);
   stencilTimeRect2->SetSortPriority(FRAME_STENCIL);
   stencilChanRect->SetSortPriority(FRAME_STENCIL);

   m_3d->AppendGeometry(stencilRect);
   m_3d->AppendGeometry(stencilTimeRect);
   //m_3d->AppendGeometry(stencilTimeRect2);
   m_3d->AppendGeometry(stencilChanRect);

   // Add a geometry at the beck of the programs panels
   m_3d->AppendGeometry(pgBackRec);
   pgBackRec->SetReflected(true);

   // Make the graph

   m_time2d->AppendChild(m_timebarNode.GetRoot());
   m_channel2d->AppendChild(m_channelbarNode.GetRoot());

   // Fill in the channel labels
   m_channelbarNode.SetInfo(info);

   SetKeepRegion(keepRegion);
   SetActiveRegion(activeRegion);
}

void Controller::Clean()
{
   PanelCache::GetPanelCache()->Clear();
   m_channelRoot.clear();
}

Controller::Controller() :
   m_root(New),
   m_3d(New),
   m_2d(New),
   m_time2d(New),
   m_channel2d(New),
   m_pastMarker(New),
   m_pastMaterial(New),
   m_longScrolling2d(New)
{
   m_channelLabelLength = Metrics::GetChannelTextWidth();
   m_timebarHeight = ChannelStride() * 0.6f;

   // Create the static part of the tree.
   m_root->AppendChild(m_3d);
   m_3d->AppendChild(m_2d);
   m_3d->AppendChild(m_time2d);
   m_3d->AppendChild(m_channel2d);
   m_3d->AppendChild(m_pastMarker);
   m_3d->AppendChild(m_longScrolling2d);

   EffectHandle   pastEffect(New);
   pastEffect->Load("past.bfx");
   m_pastMaterial->SetEffect(pastEffect);
}

void Controller::UpdateAll(const Region &activeRegion, const Region &keepRegion)
{
   SetActiveRegion(activeRegion);
   SetKeepRegion(keepRegion);
   VisibilityCull(activeRegion);
}

void Controller::VisibilityCull(const Region &region)
{
   if (Metrics::GetDoVisibilityCulling())
   {
      m_timebarNode.VisibilityCull(region);
      m_channelbarNode.VisibilityCull(region);

      for (uint32_t i = 0; i < m_channelRoot.size(); ++i)
      {
         bool  visible = region.ContainsChannel(i, 0.9999f);

         if (m_channelRoot[i].IsValid())
         {
            m_channelRoot[i].GetRoot()->SetVisible(visible);

            if (visible)
               m_channelRoot[i].VisibilityCull(region);
         }
      }
   }
}

void Controller::UpdatePastRectangle()
{
   Time now = theApp->FrameTimestamp();

   // Updates the visual rectangle that shows the past
   if (m_activeRegion.GetStartTime() < now - Time(10, Time::eSECONDS))
   {
      Vec2  bl(TimeToXCoord(m_activeRegion.GetStartTime()),
               -ChannelStride() * (m_activeRegion.GetStartIndex() + (float)m_activeRegion.GetNumChannels() - 1));
      Vec2  tr(TimeToXCoord(now),
               -ChannelStride() * (m_activeRegion.GetStartIndex() - 1.0f));

      SurfaceHandle surf = QuadFactory(bl, tr, 0.0f, eZ_AXIS).MakeSurface();

      GeometryHandle geom(New);
      geom->AppendSurfaceWithSortPriority(surf, m_pastMaterial, OVERLAY);

      m_pastMarker->ClearGeometry();
      m_pastMarker->AppendGeometry(geom);
      m_pastMarker->SetVisible(true);
   }
   else
   {
      // No rectangle needed
      m_pastMarker->ClearGeometry();
      m_pastMarker->SetVisible(false);
   }
}

void Controller::SetActiveRegion(const Region &region)
{
   m_activeRegion = region;
   UpdatePastRectangle();

   float xCentre = TimeToXCoord(region.GetStartTime() + (region.GetDuration() * 0.5f));
   float yCentre = ChannelStride() * (region.GetStartIndex() + ((float)region.GetNumChannels() * 0.5f) - 1.0f);

   m_2d->SetPosition(Vec3(-xCentre, yCentre, 0.0f));
   m_pastMarker->SetPosition(Vec3(-xCentre, yCentre, 0.0f));

   m_time2d->SetPosition(Vec3(-xCentre, ChannelStride() * (region.GetNumChannels() * 0.5f), 0.0f));

   m_channel2d->SetPosition(Vec3(-TimeToXCoord(region.GetDuration() * 0.5f) - m_channelLabelLength, yCentre, 0.0f));

   // Adjust center point to take account of channel and time bars
   m_3d->SetPosition(Vec3(m_channelLabelLength * 0.5f, 0.5f * m_timebarHeight, 0.0f));
}

void Controller::SetKeepRegion(const Region &region)
{
   m_timebarNode.SetKeepRegion(region);
   m_channelbarNode.SetKeepRegion(region);

   m_info->Lock();

   // Remove all nodes
   m_2d->ClearChildren();

#ifdef ALLOW_CHANNEL_BLIND_ROTATION
   m_panelRotGroup.Reset();
#endif

   // Remove dead channels first
   int32_t i;
   int32_t rStart = std::max(0, (int32_t)region.GetStartIndex());
   int32_t rEnd   = std::min((int32_t)m_info->GetNumChannels(), (int32_t)region.GetStartIndex() + (int32_t)region.GetNumChannels());
/*
   for (i = 0; i < rStart; i++)
      m_channelRoot[i] = ChannelNode();

   for (i = rEnd; i < (int32_t)m_info->GetNumChannels(); i++)
      m_channelRoot[i] = ChannelNode();
*/

   // Add new channels
   for (i = rStart; i < rEnd; i++)
   {
      if (!m_channelRoot[i].IsValid())
      {
         m_channelRoot[i] = ChannelNode(i, m_info->GetChannel(i));

#ifdef ALLOW_CHANNEL_BLIND_ROTATION
         m_channelRoot[i].GetRoot()->GetTransform().SetPosition(Vec3(0.0f, -0.5f, 0.0f));
#else
         m_channelRoot[i].GetRoot()->GetTransform().SetPosition(Vec3(0.0f, -i * ChannelStride(), 0.0f));
#endif
      }

#ifdef ALLOW_CHANNEL_BLIND_ROTATION
      SceneNodeHandle pos(New);
      pos->GetTransform().SetPosition(Vec3(0.0f, -i * ChannelStride() + 0.5f, 0.0f));
      SceneNodeHandle rotate(New);
      pos->AppendChild(rotate);
      rotate->AppendChild(m_channelRoot[i].GetRoot());
      m_2d->AppendChild(pos);
      m_panelRotGroup.Append(rotate->GetTransform().GetRotation());
#else
      m_2d->AppendChild(m_channelRoot[i].GetRoot());
#endif
   }

   m_info->Unlock();

   for (i = rStart; i < rEnd; i++)
   {
      // Tell the channels about the keep region
      m_channelRoot[i].SetKeepRegion(region);
   }
}

void Controller::HighlightSelection(Selection &selection, bool select)
{
   m_channelRoot[selection.GetChannel()].HighlightSelection(selection, select);
}

const Time *Controller::FindNextProgramme(Selection &selection) const
{
   return m_channelRoot[selection.GetChannel()].FindNextProgramme(selection.GetTime());
}

const Time *Controller::FindPreviousProgramme(Selection &selection) const
{
   return m_channelRoot[selection.GetChannel()].FindPreviousProgramme(selection.GetTime());
}

void Controller::UpdateFrame()
{
   UpdatePastRectangle();
}

void Controller::SetProgramsNodeCallBack(bsg::SceneNodeCallback *callBack)
{
   m_3d->SetCallback(callBack);
}

void Controller::DisplayLongScrolling(bool longScrollingOn)
{
   if (longScrollingOn)
   {
      m_longScrolling2d->SetOpacity(1);
      m_2d->SetOpacity(0);
      m_time2d->SetOpacity(0);
      m_pastMarker->SetOpacity(0);
   }
   else
   {
      m_longScrolling2d->SetOpacity(0);
      m_2d->SetOpacity(1);
      m_time2d->SetOpacity(1);
      m_pastMarker->SetOpacity(1);
   }
}
