/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "pg_channelbar_node.h"
#include "pg_info.h"
#include "pg_region.h"
#include "pg.h"

#undef min

using namespace bsg;
using namespace pg;

ChannelBarNode::ChannelBarNode() :
   m_root(bsg::New),
   m_allNodes(bsg::New),
   m_font(bsg::New),
   m_textEffect(bsg::New)
{
   m_font->Load("DroidSans-Bold.ttf", Metrics::GetChannelTextPoints());
   m_textEffect->Load("frame_text.bfx");
}

void ChannelBarNode::VisibilityCull(const Region &region)
{
   int32_t i;
   int32_t maxKids = m_allNodes->NumChildren();
   int32_t start   = std::max((int32_t)region.GetStartIndex() - 1, 0);
   int32_t end     = std::min((int32_t)region.GetStartIndex() + 2 + (int32_t)region.GetNumChannels(), maxKids);

   for (i = 0; i < start; ++i)
      m_allNodes->GetChild(i)->SetVisible(false);

   for (i = start; i < end; ++i)
      m_allNodes->GetChild(i)->SetVisible(true);

   for (i = end; i < maxKids; ++i)
      m_allNodes->GetChild(i)->SetVisible(false);
}

void ChannelBarNode::SetKeepRegion(const Region &region)
{
   m_root->ClearChildren();

   int32_t maxKids = m_allNodes->NumChildren();
   int32_t start = std::max((int32_t)region.GetStartIndex(), 0);
   int32_t end = std::min((int32_t)region.GetStartIndex() + (int32_t)region.GetNumChannels(), maxKids);

   for (int32_t i = start; i < end; ++i)
      m_root->AppendChild(m_allNodes->GetChild(i));
}

GeometryHandle ChannelBarNode::GetTextGeometry(const std::string &name)
{
   float limit = Metrics::GetChannelTextWidth();

   ShortenFormatter  *formatter = new ShortenFormatter(limit);

   PrintHandle geom(New);

   geom->SetFormatter(formatter);

   geom->SetFont(m_font, m_textEffect);
   geom->SetUniform("u_textColor", Vec4(1.0f));
   geom->SetText(name, Metrics::GetChannelTextScale(), Metrics::GetChannelTextPosition());

   return geom;
}

void ChannelBarNode::SetInfo(const GridInfo &info)
{
   info.Lock();

   // Populate the channel positions
   for (int32_t i = 0; i < (int)info.GetNumChannels(); ++i)
   {
      SceneNodeHandle node(New);

      char buf[128];
      sprintf(buf, "%03d  %s", info.GetChannel(i).GetNumber(), info.GetChannel(i).GetName().c_str());

      node->AppendGeometry(GetTextGeometry(buf));
      node->GetTransform().SetPosition(Vec3(0.0f, -i * ChannelStride(), 0.0f));

      m_allNodes->AppendChild(node);
   }

   info.Unlock();
}
