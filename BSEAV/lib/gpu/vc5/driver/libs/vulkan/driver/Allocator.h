/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include <cassert>
#include <memory>
#include <exception>
#include "vulkan.h"
#include <atomic>

namespace bvk
{

extern VkAllocationCallbacks g_defaultAllocCallbacks;

#ifndef NDEBUG
// APIScoper tracks if we are inside an API call or not.
// In a multi-threaded app, we can be in multiple APIs simultaneously.
// We use this information to check that the user-alloc-callbacks aren't
// being called outside of API functions.
class APIScoper
{
public:
   APIScoper()  { ++m_apiEntries; }
   ~APIScoper() { --m_apiEntries; }
   APIScoper(const APIScoper &rhs) = delete;
   APIScoper(APIScoper &&rhs)      = delete;

   static bool InsideAPICall() { return m_apiEntries.load() > 0; }

private:
   static std::atomic_uint_fast32_t m_apiEntries;
};
#else
// This will all get optimized away in release versions
class APIScoper
{
public:
   APIScoper()  {}
   ~APIScoper() {}
   APIScoper(const APIScoper &rhs) = delete;
   APIScoper(APIScoper &&rhs)      = delete;
   static bool InsideAPICall() { return true; }
};
#endif

// An allocator for objects of type T
// Fits the std::allocator model, so can be used in std containers for example.
// Can be rebound to allocate objects of a type different to T.
template<typename T>
class Allocator
{
public:
   using value_type        = T;
   using pointer           = T*;
   using const_pointer     = const T*;
   using reference         = T&;
   using const_reference   = const T&;
   using size_type         = std::size_t;
   using difference_type   = std::ptrdiff_t;

   Allocator() = default;

   // Constructor always needs access to the alloc callbacks and a scope
   Allocator(const VkAllocationCallbacks *cbs, VkSystemAllocationScope scope) :
      m_callbacks(cbs),
      m_scope(scope)
   {
   }

   // Allow rebinding to allocate non-T types
   template <class U>
   Allocator(const Allocator<U> &other) :
      m_callbacks(other.Callbacks()),
      m_scope(other.Scope())
   {
   }

   // std::allocate required interfaces
   T *allocate(size_t n)
   {
      assert(APIScoper::InsideAPICall()); // We must be inside an API call to use the allocator

      int s = n * sizeof(T);
      T *ptr = (T*)m_callbacks->pfnAllocation(m_callbacks->pUserData, s, alignof(T), m_scope);
      if (ptr == nullptr)
         throw std::bad_alloc();
      return ptr;
   }

   void deallocate(T* p, size_t n)
   {
      assert(APIScoper::InsideAPICall()); // We must be inside an API call to use the allocator

      if (p != nullptr)
         m_callbacks->pfnFree(m_callbacks->pUserData, (void*)p);
   }

   template<class... _Types>
   void construct(T *p, _Types&&... _Args)
   {
      new(p) T(std::forward<_Types>(_Args)...);
   }

   void destroy(T *p)
   {
      p->~T();
   }

   template <class U>
   struct rebind
   {
      typedef Allocator<U> other;
   };

   // Read-only accessors to the internal state.
   // The U-typed copy constructor needs these as it can't access T's internals.
   const VkAllocationCallbacks *Callbacks() const { return m_callbacks; }
   VkSystemAllocationScope Scope() const { return m_scope; }

private:
   const VkAllocationCallbacks   *m_callbacks = &g_defaultAllocCallbacks;
   VkSystemAllocationScope       m_scope = VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE;
};

template< class T1, class T2 >
bool operator==(const Allocator<T1>& lhs, const Allocator<T2>& rhs)
{
   return true;
}

template< class T1, class T2 >
bool operator!=(const Allocator<T1>& lhs, const Allocator<T2>& rhs)
{
   return false;
}

} // namespace bvk
