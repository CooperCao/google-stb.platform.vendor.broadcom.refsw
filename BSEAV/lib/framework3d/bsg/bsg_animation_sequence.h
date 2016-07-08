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
#ifndef __BSG_ANIMATION_SEQUENCE_H__
#define __BSG_ANIMATION_SEQUENCE_H__

#include "bsg_common.h"
#include "bsg_animator.h"
#include "bsg_animation_list.h"

#include <vector>

/** @addtogroup animation
 @{
 */

namespace bsg
{

class AnimationSequence;

// @cond

// Base class for animation sequence nodes
// These classes hold the actions in the animation seuqnce
class SequenceNode
{
public:
   SequenceNode(AnimationSequence *owner) : m_owner(owner) {}
   virtual ~SequenceNode();

   virtual void Execute(const Time &startTime) = 0;
   virtual void Stop() = 0;

protected:
   AnimationSequence *m_owner;
};

// Runs the specified animation for a given duration
class RunAnimSequenceNode : public SequenceNode
{
public:
   RunAnimSequenceNode(AnimationSequence *owner, AnimBindingBase *anim, float durationSeconds);
   virtual ~RunAnimSequenceNode();
   virtual void Execute(const Time &startTime);
   virtual void Stop();
private:
   AnimBindingBase *m_anim;
   float           m_duration;
};

// Transfers control to the label
class GotoSequenceNode : public SequenceNode
{
public:
   GotoSequenceNode(AnimationSequence *owner, uint32_t label);
   virtual void Execute(const Time &startTime);
   virtual void Stop();

private:
   uint32_t m_label;
};

// Waits for a delay
class DelaySequenceNode : public SequenceNode
{
public:
   DelaySequenceNode(AnimationSequence *owner, float durationSecs);
   ~DelaySequenceNode();
   virtual void Execute(const Time &startTime);
   virtual void Stop();

private:
   float                m_duration;
   AnimBindingLerpFloat *m_anim;
   AnimatableFloat      m_dummyTarget;
};

// Repeat a loop for a specified number of iterations
class LoopSequenceNode : public SequenceNode
{
public:
   LoopSequenceNode(AnimationSequence *owner, uint32_t label, uint32_t loopCount);
   virtual void Execute(const Time &startTime);
   virtual void Stop();

private:
   uint32_t m_label;
   uint32_t m_loopCount;
   uint32_t m_curLoop;
};

//! It is often necessary to chain together a sequence of animations.  Applications can do this manually by waiting for each
//! animation to complete and then adding a new animation into its animation list.  Alternatively the AnimationSequence class
//! gives an automated method of achieving the same effect automatically.
class AnimationSequence : public AnimationDoneNotifier
{
public:
   //! Create an empty, uninitialised sequence.
   AnimationSequence();

   //! Creates an empty sequence which is linked to the specified animation list.  Animations will be placed into this list
   //! as the sequence is progressed.
   AnimationSequence(AnimationList *animList);

   virtual ~AnimationSequence();

   //! Re-initialist an animation list
   void Init(AnimationList *animList);

   //! Adds the specified animation to the sequence.
   void AppendAnimation(AnimBindingBase *animation, float durationSeconds);

   //! Get the label for the next slot
   uint32_t AppendLabel();

   //! Add an unconditional jump to the sequence
   void AppendGoto(uint32_t label);

   //! Add a counted loop to the sequence
   void AppendLoop(uint32_t label, uint32_t loopCount);

   //! Add a delay to the sequence
   void AppendDelay(float secs);

   //! Start the sequence
   void Start(const Time &startTime);

   //! Stop the sequence, removing all animations from the anim list
   void Stop();

   friend class LoopSequenceNode;
   friend class GotoSequenceNode;
   friend class RunAnimSequenceNode;
   friend class DelaySequenceNode;

protected:
   AnimationList *AnimList() { return m_animList; }
   virtual void Notify(const Time &time);
   void Goto(uint32_t label, const Time &t);

private:
   AnimationList              *m_animList;
   std::vector<SequenceNode*> m_sequence;
   uint32_t                   m_curAnim;
};


}

/** @} */

#endif /* __BSG_ANIMATION_SEQUENCE_H__ */
