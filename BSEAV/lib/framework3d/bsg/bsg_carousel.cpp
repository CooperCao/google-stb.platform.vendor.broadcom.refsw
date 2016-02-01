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

#include "bsg_carousel.h"


namespace bsg
{

Carousel::Carousel(bool fadeIn, bool fadeOut) :
   m_dirty(true),
   m_animatingNow(false),
   m_fadeIn(fadeIn),
   m_fadeOut(fadeOut),
   m_loop(false),
   m_select(-1),
   m_root(New),
   m_wrapCallback(0)
{
}

void Carousel::EmptyRoot()
{
   m_root->ClearChildren();
}

void Carousel::EmptyGeom()
{
   m_geomNodes.clear();
}

Carousel::~Carousel()
{
   EmptyRoot();
   EmptyGeom();
   delete m_wrapCallback;
}

void Carousel::AddGeometry(GeometryHandle geom)
{
   SceneNodeHandle node(New);
   node->AppendGeometry(geom);
   m_geomNodes.push_back(node);
   m_dirty = true;
}

void Carousel::AddGeometry(SceneNodeHandle geomNode)
{
   // Extra node is so that all geom nodes are owned by us (although their
   // children will not be)
   SceneNodeHandle node(New);
   node->AppendChild(geomNode);
   m_geomNodes.push_back(node);
   m_dirty = true;
}

void Carousel::Prepare()
{
   if (m_dirty)
   {
      // By default choose the middle item
      uint32_t select = m_select == -1 ? NumElements() / 2 : m_select;

      m_startNodeIndex = CircularIndex(0, NumElements() - 1);

      CircularIndex indx = CircularIndex(0, m_geomNodes.size() - 1);

      if (m_loop)
         indx--;

      m_selGeomIndex = indx;
      m_startGeomIndex = indx;

      for (uint32_t n = 0; n < NumElements(); n++)
      {
         SceneNodeHandle node = ElementNode(n);
         node->ClearChildren();
         node->AppendChild(m_geomNodes[indx]);

         if (n == select)
         {
            m_selGeomIndex = indx;
            if (m_loop)
               m_selGeomIndex++;
         }

         indx.Increment();
      }

      ElementNode(0)->SetVisible(false);
      ElementNode(NumElements() - 1)->SetVisible(false);

      m_endGeomIndex = indx.Minus1();

      m_dirty = false;
   }
}

const SceneNodeHandle Carousel::RootNode()
{
   return m_root;
}

SceneNodeHandle Carousel::ElementNode(uint32_t n)
{
   return m_root->GetChild(n);
}

uint32_t Carousel::NumElements() const
{
   return m_root->NumChildren();
}

void Carousel::Prev(Time now, Time animationTime)
{
   if (m_animatingNow)
      return;

   m_animatingNow = true;

   CircularIndex s = m_startNodeIndex;
   CircularIndex e = m_startNodeIndex.Minus1();

   // Remember the start transform
   Transform startTx = ElementNode(s)->GetTransform();

   for (uint32_t n = 0; n < NumElements() - 1; n++)
   {
      CircularIndex next = s.Plus1();

      AnimBindingEval<TransformEvaluator>   *anim = MakeTransition(&ElementNode(s)->GetTransform());
      anim->Init(ElementNode(s)->GetTransform(), now.Seconds(), 
                 ElementNode(next)->GetTransform(), (now + animationTime).Seconds());
      m_animList.Append(anim);

      ElementNode(s)->SetVisible(true);
      ElementNode(s)->SetOpacity(1.0f);

      s++;
   }

   // Move the last block to the first block without animating (and set its geom)
   SceneNodeHandle geom = m_geomNodes[m_startGeomIndex.Minus1()];

   ElementNode(e)->SetTransform(startTx);
   ElementNode(e)->ClearChildren();
   ElementNode(e)->AppendChild(geom);
   ElementNode(e)->SetVisible(false);

   if (m_wrapCallback != 0)
      m_wrapCallback->Prev(geom->GetChild(0));

   // Configure fade in & out of outer elements
   if (m_fadeOut)
   {
      AnimBindingHermiteFloat *animOut = new AnimBindingHermiteFloat;
      animOut->Init(1.0f, now, 0.0f, now + animationTime);
      animOut->Bind(&ElementNode(e.Minus1())->GetOpacity());
      m_animList.Append(animOut);
   }

   if (m_fadeIn)
   {
      AnimBindingHermiteFloat *animIn = new AnimBindingHermiteFloat;
      animIn->Init(0.0f, now, 1.0f, now + animationTime);
      animIn->Bind(&ElementNode(s.Plus1())->GetOpacity());
      m_animList.Append(animIn);
   }

   m_startNodeIndex--;
   m_startGeomIndex--;
   m_endGeomIndex--;
   m_selGeomIndex--;
}

void Carousel::Next(Time now, Time animationTime)
{
   if (m_animatingNow)
      return;

   m_animatingNow = true;

   CircularIndex s = m_startNodeIndex.Minus1();
   CircularIndex e = m_startNodeIndex;

   // Remember the start transform
   Transform startTx = ElementNode(s)->GetTransform();

   for (uint32_t n = 0; n < NumElements() - 1; n++)
   {
      CircularIndex next = s.Minus1();

      AnimBindingEval<TransformEvaluator>   *anim = MakeTransition(&ElementNode(s)->GetTransform());
      anim->Init(ElementNode(s)->GetTransform(),    now, 
                 ElementNode(next)->GetTransform(), now + animationTime);
      m_animList.Append(anim);

      ElementNode(s)->SetVisible(true);
      ElementNode(s)->SetOpacity(1.0f);

      s--;
   }

   // Move the last block to the first block without animating
   SceneNodeHandle   geom = m_geomNodes[m_endGeomIndex.Plus1()];

   ElementNode(e)->SetTransform(startTx);
   ElementNode(e)->ClearChildren();
   ElementNode(e)->AppendChild(geom);
   ElementNode(e)->SetVisible(false);

   if (m_wrapCallback != 0)
      m_wrapCallback->Next(geom->GetChild(0));

   // Configure fade in & out of outer elements
   if (m_fadeOut)
   {
      AnimBindingHermiteFloat *animOut = new AnimBindingHermiteFloat(&ElementNode(s.Plus1())->GetOpacity());
      animOut->Init(1.0f, now, 0.0f, now + animationTime);
      m_animList.Append(animOut);
   }

   if (m_fadeIn)
   {
      AnimBindingHermiteFloat *animIn = new AnimBindingHermiteFloat(&ElementNode(e.Minus1())->GetOpacity());
      animIn->Init(0.0f, now, 1.0f, now + animationTime);
      m_animList.Append(animIn);
   }

   m_startNodeIndex++;
   m_startGeomIndex++;
   m_endGeomIndex++;
   m_selGeomIndex++;
}

bool Carousel::UpdateAnimationList(Time t)
{
   Prepare();
   m_animatingNow = m_animList.UpdateTime(t);
   return m_animatingNow;
}

uint32_t Carousel::CurrentIndex() const
{
   return m_selGeomIndex.Current();
}

}

