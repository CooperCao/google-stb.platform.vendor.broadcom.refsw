/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
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
