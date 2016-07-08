/*=============================================================================
Broadcom Proprietary and Confidential. (c)2011 Broadcom.
All rights reserved.

Project  :  BSG
Module   :  Maybe

FILE DESCRIPTION
Utility class of a possible T
=============================================================================*/

#ifndef __BSG_MAYBE_H__
#define __BSG_MAYBE_H__

#include <vector>

namespace bsg
{

// @cond
template <class T>
class Maybe
{
public:
   operator bool() const         { return m_value.size() > 0;  }
   const T  &Get() const         { return m_value[0];          }
   void     Set(const T &value)  { m_value.push_back(value);   }

private:
   std::vector<T> m_value;
};

// @endcond
}

#endif /* __BSG_MAYBE_H__ */

