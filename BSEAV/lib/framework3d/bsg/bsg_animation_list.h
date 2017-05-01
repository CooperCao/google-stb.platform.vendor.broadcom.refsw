/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __BSG_ANIMATION_LIST_H__
#define __BSG_ANIMATION_LIST_H__

#include "bsg_no_copy.h"
#include "bsg_common.h"
#include "bsg_time.h"
#include <set>

namespace bsg
{

class AnimBindingBase;

/** @addtogroup animation
 @{
 */

//! This class holds a set of animation bindings.
//! Everything in the set will be updated at the same time by using the UpdateTime method.
//! Applications create one or more lists and call UpdateTime during the application UpdateFrame method.
class AnimationList : public NoCopy
{
public:
   AnimationList();
   virtual ~AnimationList();

   //! Add a new animation binding to the list
   void Append(AnimBindingBase *binding);

   //! Clear all animation bindings
   void Clear();

   //! Delete a specific binding
   void Delete(AnimBindingBase *item);

   //! Update all animations in the list to the given bsg::Time.
   //! Returns true if something was animated, false if everything is finished or not yet started.
   bool UpdateTime(Time time);

   //! Returns the number of milliseconds until the start of the next animation.
   //! Returns 0 if something is already animating.
   //! Returns -1 if everything has finished, and nothing is pending.
   int64_t TimeToStartMs(Time time) const;

private:
   std::set<AnimBindingBase *>  m_set;
};

/** @} */

}

#endif /* __BSG_ANIMATION_LIST_H__ */
