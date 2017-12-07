/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include "vulkan.h"
#include "Allocator.h"
#include <vector>
#include <list>
#include <memory>
#include <queue>
#include <deque>
#include <map>
#include <forward_list>
#include <set>
#include <string>
#include <functional>
#include <utility>

namespace bvk
{

// The base class for all objects which will be allocating memory, or using
// std container classes. Use of this base class ensures system memory requests
// are passed to the application supplied callbacks.
class Allocating
{
public:
   Allocating(const VkAllocationCallbacks *cbs) :
      m_allocCallbacks(cbs)
   {
   }

   // Return an allocator for type T
   template <class T>
   Allocator<T> GetAllocator(VkSystemAllocationScope scope)
   {
      return Allocator<T>(m_allocCallbacks, scope);
   }

   // This case is so common, we'll make it easier to use.
   // Return an allocator for type T using VK_SYSTEM_ALLOCATION_SCOPE_OBJECT
   template <class T>
   Allocator<T> GetObjScopeAllocator()
   {
      return Allocator<T>(m_allocCallbacks, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   }

   const VkAllocationCallbacks  *GetCallbacks() const { return m_allocCallbacks; }

   // Ensure nothing tries to use new and delete
   void *operator new(size_t count) = delete;
   void *operator new[](size_t count) = delete;
   void operator delete(void *p) { assert(0); } // Delete is required, even if not used!
   void operator delete[](void *p) = delete;

   // We still need placement new & delete to work
   void *operator new(size_t count, void *ptr)     { return ::operator new(count, ptr);   }
   void *operator new[](size_t count, void *ptr)   { return ::operator new[](count, ptr); }
   void operator delete(void *ptr, void *place)    { ::operator delete(ptr, place);       }
   void operator delete[](void *ptr, void *place)  { ::operator delete[](ptr, place);     }

private:
   const VkAllocationCallbacks  *m_allocCallbacks;
};

template<class T, VkSystemAllocationScope s, class... Types>
inline T *createObject(const VkAllocationCallbacks *cbs,
                       const VkAllocationCallbacks *fallback,
                       Types&&... args)
{
   if (cbs == nullptr)
      cbs = fallback;

   Allocator<T> alloc(cbs, s);

   void *ptr = alloc.allocate(1);
   if (ptr == nullptr)
      throw std::bad_alloc();

   T *ret = nullptr;
   try
   {
      ret = new (ptr)T(cbs, std::forward<Types>(args)...);
   }
   catch (...)
   {
      alloc.deallocate(static_cast<T*>(ptr), sizeof(T));
      throw;
   }
   return ret;
}

template <VkSystemAllocationScope s, class T>
inline void destroyObject(T *obj, const VkAllocationCallbacks* cbs)
{
   // All vkDestroyXXX API entrypoints are allowed to pass NULL/VK_NULL_OBJECT.
   // However the spec makes no clear indication as to if a user defined free
   // callback should get called in this case (other than it would be safe
   // to do so).
   //
   // The current decision is not to do that and just do nothing
   if (obj == nullptr)
      return;

   if (cbs == nullptr)
      cbs = obj->GetCallbacks();

   Allocator<T> a(cbs, s);
   a.destroy(obj);
   a.deallocate(obj, sizeof(T));
}

// Templates that make using std containers easier with our allocators

template<typename T>
using vector = std::vector<T, Allocator<T>>;

template<typename T>
using list   = std::list<T, Allocator<T>>;

template<typename T>
using deque  = std::deque<T, Allocator<T>>;

template<typename T>
using queue  = std::queue<T, bvk::deque<T>>;

template<typename T>
using priority_queue = std::priority_queue<T, Allocator<T>>;

template<typename T>
using forward_list = std::forward_list<T, Allocator<T>>;

template<typename Key, class Compare = std::less<Key>>
using set = std::set<Key, Compare, Allocator<Key>>;

template<typename Key, typename T, class Compare = std::less<Key>>
using map = std::map<Key, T, Compare, Allocator<std::pair<const Key, T>>>;

template<typename Key, typename T, class Compare = std::less<Key>>
using multimap = std::multimap<Key, T, Compare, Allocator<std::pair<const Key, T>>>;

using string = std::basic_string<char, std::char_traits<char>, Allocator<char>>;

template<class _Ty, class... _Types> inline
std::shared_ptr<_Ty> allocate_shared(const Allocator<_Ty>& _Al_arg, _Types&&... _Args)
{
   return std::allocate_shared<_Ty, Allocator<_Ty>,  _Types&&...>(_Al_arg, std::forward<_Types>(_Args)...);
}

} // namespace bvk
