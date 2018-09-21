/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include "ArenaAllocator.h"
#include "SysMemCmdBlock.h"

#include <map>
#include <vector>
#include <list>
#include <cstring>

namespace spv {

extern void LogUsage(const char *name, const bvk::ArenaAllocator<bvk::SysMemCmdBlock, void*> &arena);

//////////////////////////////////////////////////////////////////////////////
// PoolAllocator
//
// An templated arena allocator for objects of type T.
// Fits the std::allocator model, so can be used in std containers for example.
// Can be rebound to allocate objects of a type different to T.
//
// Note: There is no deallocation in this allocator. All allocations are
// destroyed only when the arena is destroyed. Take care to ensure that std
// containers are sized as best as possible in advance to avoid using more
// memory than you need. Using push_back() on vectors that haven't first been
// 'reserve()'d is not wise.
//////////////////////////////////////////////////////////////////////////////
template<typename T>
class PoolAllocator
{
public:
   PoolAllocator() {}

   PoolAllocator(bvk::ArenaAllocator<bvk::SysMemCmdBlock, void*> *arena) :
      m_arena(arena)
   {
   }

   // Allow rebinding to allocate non-T types
   template <class U>
   PoolAllocator(const PoolAllocator<U> &other) :
      m_arena(other.Arena())
   {
   }

   // Utility for constructing objects of type U in the Pool
   template <class U, class... Types>
   U *New(Types&&... args) const
   {
      U *p = PoolAllocator<U>(*this).allocate(1);
      return new (p) U(std::forward<Types>(args)...);
   }

   // Utility for constructing arrays of objects of type U in the Pool
   template <class U>
   U *NewArray(uint32_t numElems) const
   {
      U *p = PoolAllocator<U>(*this).allocate(numElems);

      for (uint32_t i = 0; i < numElems; ++i)
         new (p + i) U();

      return p;
   }

   ////////////////////////////////////////////////////////////////////////////////////////////////
   // std::allocate required interfaces
   ////////////////////////////////////////////////////////////////////////////////////////////////
   using value_type = T;
   using pointer = T*;
   using const_pointer = const T*;
   using reference = T&;
   using const_reference = const T&;
   using size_type = std::size_t;
   using difference_type = std::ptrdiff_t;

   T *allocate(size_t n) const
   {
      size_t s = n * sizeof(T);

      void *ptr;
      m_arena->Allocate(&ptr, s, 4);
      if (ptr == nullptr)
         throw std::bad_alloc();
      return static_cast<T*>(ptr);
   }

   void deallocate(T* p, size_t n) const
   {
      // No deallocation in an arena allocator
      m_arena->RecordWastedDelete(n * sizeof(T));
   }

   template<class... _Types>
   void construct(T *p, _Types&&... _Args)
   {
      new (p) T(std::forward<_Types>(_Args)...);
   }

   void destroy(T *p)
   {
      p->~T();
   }

   template <class U>
   struct rebind
   {
      typedef PoolAllocator<U> other;
   };

   void LogMemoryUsage(const char *name) const
   {
      LogUsage(name, *m_arena);
   }

   // Read-only accessor to the internal state.
   // The U-typed copy constructor needs these as it can't access T's internals.
   bvk::ArenaAllocator<bvk::SysMemCmdBlock, void*> *Arena() const { return m_arena; }

private:
   bvk::ArenaAllocator<bvk::SysMemCmdBlock, void*>  *m_arena = nullptr;
   mutable size_t                                    m_wastedDeletedBytes = 0;
};

template< class T1, class T2 >
bool operator==(const PoolAllocator<T1>& lhs, const PoolAllocator<T2>& rhs)
{
   return true;
}

template< class T1, class T2 >
bool operator!=(const PoolAllocator<T1>& lhs, const PoolAllocator<T2>& rhs)
{
   return false;
}

// Some definitions that make it easier to use std containers.
// Use spv::vector rather than std::vector for example.
template<typename T>
using vector = std::vector<T, PoolAllocator<T>>;

template<typename T>
using list = std::list<T, PoolAllocator<T>>;

template<typename T>
using forward_list = std::forward_list<T, PoolAllocator<T>>;

template<typename Key, class Compare = std::less<Key>>
using set = std::set<Key, Compare, PoolAllocator<Key>>;

template<typename Key, typename T, class Compare = std::less<Key>>
using map = std::map<Key, T, Compare, PoolAllocator<std::pair<const Key, T>>>;

template<typename Key, typename T, class Compare = std::less<Key>>
using multimap = std::multimap<Key, T, Compare, PoolAllocator<std::pair<const Key, T>>>;

using string = std::basic_string<char, std::char_traits<char>, PoolAllocator<char>>;
inline bool operator==(const spv::string &lhs, const std::string &rhs)
{
   return strcmp(lhs.c_str(), rhs.c_str()) == 0;
}
inline bool operator!=(const spv::string &lhs, const std::string &rhs)
{
   return strcmp(lhs.c_str(), rhs.c_str()) != 0;
}
inline bool operator==(const spv::string &lhs, const char *rhs)
{
   return strcmp(lhs.c_str(), rhs) == 0;
}
inline bool operator!=(const spv::string &lhs, const char *rhs)
{
   return strcmp(lhs.c_str(), rhs) != 0;
}

} // namespace spv

namespace bvk
{

using SpvAllocator = spv::PoolAllocator<uint32_t>;

}
