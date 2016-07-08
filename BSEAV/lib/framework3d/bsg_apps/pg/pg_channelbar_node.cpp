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
