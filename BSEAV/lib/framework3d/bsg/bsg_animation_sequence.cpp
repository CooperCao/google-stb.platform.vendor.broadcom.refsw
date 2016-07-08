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

#include "bsg_animation_sequence.h"

namespace bsg
{
   ///////////////////////////////////////////////////////////////////////////
   SequenceNode::~SequenceNode()
   {
   }
   ///////////////////////////////////////////////////////////////////////////

   ///////////////////////////////////////////////////////////////////////////
   RunAnimSequenceNode::RunAnimSequenceNode(AnimationSequence *owner, AnimBindingBase *anim, float durationSeconds) :
   SequenceNode(owner),
      m_anim(anim),
      m_duration(durationSeconds)
   {
   }

   RunAnimSequenceNode::~RunAnimSequenceNode()
   {
      m_owner->AnimList()->Delete(m_anim);
      delete m_anim;
   }

   void RunAnimSequenceNode::Execute(const Time &startTime)
   {
      m_anim->SetDeleteWhenDone(false);
      m_anim->GetBaseInterpolator()->SetAnimationDoneNotifier(m_owner);
      m_anim->GetBaseInterpolator()->SetMode(BaseInterpolator::eLIMIT);
      m_anim->GetBaseInterpolator()->SetStartAndDuration(startTime.Seconds(), m_duration);
      m_owner->AnimList()->Append(m_anim);
   }

   void RunAnimSequenceNode::Stop()
   {
      m_owner->AnimList()->Delete(m_anim);
   }

   ///////////////////////////////////////////////////////////////////////////

   ///////////////////////////////////////////////////////////////////////////
   GotoSequenceNode::GotoSequenceNode(AnimationSequence *owner, uint32_t label) :
   SequenceNode(owner),
      m_label(label)
   {
   }

   void GotoSequenceNode::Execute(const Time &startTime)
   {
      m_owner->Goto(m_label, startTime);
   }

   void GotoSequenceNode::Stop()
   {
   }

   ///////////////////////////////////////////////////////////////////////////

   ///////////////////////////////////////////////////////////////////////////
   LoopSequenceNode::LoopSequenceNode(AnimationSequence *owner, uint32_t label, uint32_t loopCount) :
   SequenceNode(owner),
      m_label(label),
      m_loopCount(loopCount),
      m_curLoop(0)
   {
   }

   void LoopSequenceNode::Execute(const Time &startTime)
   {
      m_curLoop++;
      if (m_curLoop < m_loopCount)
         m_owner->Goto(m_label, startTime);
      else
         m_owner->Notify(startTime);
   }

   void LoopSequenceNode::Stop()
   {
   }

   ///////////////////////////////////////////////////////////////////////////

   ///////////////////////////////////////////////////////////////////////////
   DelaySequenceNode::DelaySequenceNode(AnimationSequence *owner, float durationSecs) :
   SequenceNode(owner),
      m_duration(durationSecs)
   {
      m_anim = new AnimBindingLerpFloat;
      m_anim->Evaluator()->Init(0.0f, durationSecs);
      m_anim->Bind(&m_dummyTarget);
   }

   DelaySequenceNode::~DelaySequenceNode()
   {
      m_owner->AnimList()->Delete(m_anim);
      delete m_anim;
   }

   void DelaySequenceNode::Execute(const Time &startTime)
   {
      m_anim->SetDeleteWhenDone(false);
      m_anim->GetBaseInterpolator()->SetAnimationDoneNotifier(m_owner);
      m_anim->GetBaseInterpolator()->SetMode(BaseInterpolator::eLIMIT);
      m_anim->GetBaseInterpolator()->SetStartAndDuration(startTime.Seconds(), m_duration);
      m_owner->AnimList()->Append(m_anim);
   }

   void DelaySequenceNode::Stop()
   {
      m_owner->AnimList()->Delete(m_anim);
   }

   ///////////////////////////////////////////////////////////////////////////

   ///////////////////////////////////////////////////////////////////////////
   AnimationSequence::AnimationSequence(AnimationList *animList) :
      m_animList(animList),
      m_curAnim(0)
   {
   }

   AnimationSequence::AnimationSequence() :
      m_animList(NULL),
      m_curAnim(0)
   {
   }

   void AnimationSequence::Init(AnimationList *animList)
   {
      m_animList = animList;
      m_curAnim = 0;
   }

   AnimationSequence::~AnimationSequence()
   {
      std::vector<SequenceNode*>::iterator  iter;
      for (iter = m_sequence.begin(); iter != m_sequence.end(); ++iter)
         delete (*iter);

      m_sequence.clear();
   }

   void AnimationSequence::AppendAnimation(AnimBindingBase *animation, float durationSeconds)
   {
      m_sequence.push_back(new RunAnimSequenceNode(this, animation, durationSeconds));
   }

   uint32_t AnimationSequence::AppendLabel()
   {
      return m_sequence.size();
   }

   void AnimationSequence::AppendGoto(uint32_t label)
   {
      m_sequence.push_back(new GotoSequenceNode(this, label));
   }

   void AnimationSequence::AppendLoop(uint32_t label, uint32_t loopCount)
   {
      m_sequence.push_back(new LoopSequenceNode(this, label, loopCount));
   }

   void AnimationSequence::AppendDelay(float secs)
   {
      m_sequence.push_back(new DelaySequenceNode(this, secs));
   }

   void AnimationSequence::Start(const Time &startTime)
   {
      SequenceNode *node = m_sequence.front();
      if (node)
         node->Execute(startTime);
   }

   void AnimationSequence::Stop()
   {
      if (m_curAnim < m_sequence.size())
         m_sequence[m_curAnim]->Stop();
   }

   void AnimationSequence::Notify(const Time &time)
   {
      m_curAnim++;

      if (m_curAnim < m_sequence.size())
         m_sequence[m_curAnim]->Execute(time);
   }

   void AnimationSequence::Goto(uint32_t label, const Time &t)
   {
      if (label < m_sequence.size())
      {
         m_curAnim = label;
         m_sequence[m_curAnim]->Execute(t);
      }
   }


   ///////////////////////////////////////////////////////////////////////////

}

