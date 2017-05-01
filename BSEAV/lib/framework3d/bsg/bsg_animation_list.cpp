/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
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

void AnimationList::Append(AnimBindingBase *binding)
{
   m_set.insert(binding);
}

void AnimationList::Clear()
{
   std::set<AnimBindingBase *>::iterator iter;

   for (iter = m_set.begin(); iter != m_set.end(); ++iter)
      if ((*iter)->DeleteWhenDone())
         delete *iter;

   m_set.clear();
}

bool AnimationList::UpdateTime(Time time)
{
   bool anyLive = false;

   std::set<AnimBindingBase *>::iterator iter;
   std::list<AnimBindingBase *>          deleteList;

   for (iter = m_set.begin(); iter != m_set.end(); ++iter)
   {
      anyLive = (*iter)->UpdateTime(time.Seconds()) || anyLive;
      if ((*iter)->Finished(time.Seconds()))
         deleteList.push_back(*iter);
   }

   std::list<AnimBindingBase *>::iterator delIter;
   for (delIter = deleteList.begin(); delIter != deleteList.end(); ++delIter)
   {
      iter = m_set.find(*delIter);
      if ((*iter)->DeleteWhenDone())
         delete *iter;
      m_set.erase(iter);
   }

   return anyLive;
}

void AnimationList::Delete(AnimBindingBase *item)
{
   std::set<AnimBindingBase *>::iterator iter = m_set.find(item);

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
   std::set<AnimBindingBase *>::iterator iter;

   for (iter = m_set.begin(); iter != m_set.end(); ++iter)
   {
      int64_t ms = (*iter)->TimeToStartMs(time);
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
