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

#include "bsg_animation_list.h"
#include "bsg_animator.h"

#undef min
#undef max

#include <list>

namespace bsg
{

AnimationList::AnimationList()
{
}

AnimationList::~AnimationList()
{
   Clear();
}

void AnimationList::Append(AnimBindingBase *anim)
{
   m_set.insert(anim);
}

void AnimationList::Clear()
{
   for (auto anim : m_set)
      if (anim->DeleteWhenDone())
         delete anim;

   m_set.clear();
}

bool AnimationList::UpdateTime(Time time)
{
   bool anyLive = false;
   std::list<AnimBindingBase *>          deleteList;

   for (auto anim : m_set)
   {
      bool isLive = anim->UpdateTime(time.Seconds());

      anyLive = anyLive || isLive;

      if (anim->Finished(time.Seconds()))
         deleteList.push_back(anim);
   }

   for (auto anim : deleteList)
   {
      auto iter = m_set.find(anim);
      if ((*iter)->DeleteWhenDone())
         delete *iter;
      m_set.erase(iter);
   }

   return anyLive;
}

void AnimationList::Delete(AnimBindingBase *item)
{
   auto iter = m_set.find(item);

   if (iter != m_set.end())
   {
      if ((*iter)->DeleteWhenDone())
         delete *iter;
      m_set.erase(iter);
   }
}

int64_t AnimationList::TimeToStartMs(Time time) const
{  
   int64_t                               minMs = -1;

   for (auto anim : m_set)
   {
      int64_t ms = anim->TimeToStartMs(time);
      if (ms == 0)
         return 0;
      else if (ms > 0)
      {
         if (minMs > 0)
            minMs = std::min(minMs, ms);
         else
            minMs = ms;
      }
   }

   return minMs;
}

}

