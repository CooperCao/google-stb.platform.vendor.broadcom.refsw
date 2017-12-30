/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once
#include <algorithm>
#include "libs/util/common.h"

#define template_T_Tag template<typename T, typename Tag>
#define template_T_Tag_D template_T_Tag template<typename D>

// Private API
//-----------------------------------------------------------------------------

template_T_Tag inline void intrusive_list<T,Tag>::init()
{
   m_head.next = &m_head;
   m_head.prev = &m_head;
   m_head.set_list(this);
}

// Assert that h is in a list but is not the first in the list.
template_T_Tag inline void intrusive_list<T,Tag>::check_not_begin(hook* h)
{
#ifndef NDEBUG
   assert(h->owner != NULL && h->prev != &h->owner->m_head);
#endif
}

// Assert that h is in a list but is not the special end hook (m_head).
template_T_Tag inline void intrusive_list<T,Tag>::check_not_end(hook* h)
{
#ifndef NDEBUG
   assert(h->owner != NULL && h != &h->owner->m_head);
#endif
}

template_T_Tag inline auto intrusive_list<T,Tag>::from_hook(hook* h) -> T*
{
   check_not_end(h);
   assume(h != nullptr);
   return static_cast<T*>(h);
}

// Assert that this iterator is valid for this list.
template_T_Tag inline void intrusive_list<T,Tag>::check_iterator(const_iterator const& i)
{
   i.h->check_list(this);
}

// Public API
//-----------------------------------------------------------------------------

template_T_Tag inline intrusive_list<T,Tag>::intrusive_list()
{
   init();
}

template_T_Tag inline intrusive_list<T,Tag>::intrusive_list(intrusive_list&& other)
{
   init();
   *this = std::move(other);
}

template_T_Tag auto intrusive_list<T,Tag>::operator=(intrusive_list&& other) -> intrusive_list&
{
   assert(empty()); // can only assign to empty list

   // move pointers over, fixing up the head pointers
   if (other.m_head.next != &other.m_head)
   {
      if (IS_DEBUG)
      {
         // update list pointers in each node
         for (hook* h = other.m_head.next; h != &other.m_head; h = h->next)
         {
            h->clear_list(&other);
            h->set_list(this);
         }
      }

      m_head.next = other.m_head.next;
      m_head.prev = other.m_head.prev;
      m_head.next->prev = &m_head;
      m_head.prev->next = &m_head;

      // reset other
      other.m_head.next = &other.m_head;
      other.m_head.prev = &other.m_head;
   }
   return *this;
}

template_T_Tag inline intrusive_list<T,Tag>::~intrusive_list()
{
   assert(empty());
   m_head.clear_list(this);
}

template_T_Tag inline void intrusive_list<T,Tag>::clear()
{
   // in debug, nodes need to be told they are off the list
   while (IS_DEBUG && !empty())
      pop_front();
   m_head.next = &m_head;
   m_head.prev = &m_head;
}

template_T_Tag void intrusive_list<T,Tag>::swap(intrusive_list& other)
{
   bool this_empty = this->empty();
   bool other_empty = other.empty();

   if (IS_DEBUG)
   {
      // update list pointers in each node
      for (hook* h = this->m_head.next; h != &this->m_head; h = h->next)
      {
         h->clear_list(this);
         h->set_list(&other);
      }
      for (hook* h = other.m_head.next; h != &other.m_head; h = h->next)
      {
         h->clear_list(&other);
         h->set_list(this);
      }
   }
   std::swap(m_head.next, other.m_head.next);
   std::swap(m_head.prev, other.m_head.prev);

   // Head next and prev already swapped so to update
   // this->m_head.next/prev check emptiness of other
   if (other_empty)
   {
      m_head.next = &m_head;
      m_head.prev = &m_head;
   }
   else
   {
      m_head.next->prev = &m_head;
      m_head.prev->next = &m_head;
   }

   // Head next and prev already swapped so to update
   // other.m_head.next/prev check emptiness of this
   if (this_empty)
   {
      other.m_head.next = &other.m_head;
      other.m_head.prev = &other.m_head;
   }
   else
   {
      other.m_head.next->prev = &other.m_head;
      other.m_head.prev->next = &other.m_head;
   }
}

template_T_Tag inline bool intrusive_list<T,Tag>::empty() const
{
   assert((m_head.next == &m_head) == (m_head.prev == &m_head));
   return m_head.next == &m_head;
}

template_T_Tag inline size_t intrusive_list<T,Tag>::size() const
{
   size_t size = 0;
   for (hook* h = m_head.next; h != &m_head; h = h->next, ++size);
   return size;
}

template_T_Tag inline T& intrusive_list<T,Tag>::push_front(T& val)
{
   hook* val_hook = &val;
   val_hook->set_list(this);
   val_hook->next = m_head.next;
   val_hook->prev = &m_head;
   m_head.next->prev = val_hook;
   m_head.next = val_hook;
   return val;
}

template_T_Tag inline T& intrusive_list<T,Tag>::push_back(T& val)
{
   hook* val_hook = &val;
   val_hook->set_list(this);
   val_hook->next = &m_head;
   val_hook->prev = m_head.prev;
   m_head.prev->next = val_hook;
   m_head.prev = val_hook;
   return val;
}

template_T_Tag inline T& intrusive_list<T,Tag>::pop_front()
{
   assert(!empty());
   hook* h = m_head.next;
   T* p = from_hook(h);
   h->clear_list(this);
   m_head.next = h->next;
   m_head.next->prev = &m_head;
   return *p;
}

template_T_Tag inline T& intrusive_list<T,Tag>::pop_back()
{
   assert(!empty());
   hook* h = m_head.prev;
   T* p = from_hook(h);
   h->clear_list(this);
   m_head.prev = h->prev;
   m_head.prev->next = &m_head;
   return *p;
}

template_T_Tag inline T& intrusive_list<T,Tag>::front()
{
   assert(!empty());
   return *from_hook(m_head.next);
}

template_T_Tag inline T const& intrusive_list<T,Tag>::front() const
{
   assert(!empty());
   return *from_hook(m_head.next);
}

template_T_Tag inline auto intrusive_list<T,Tag>::begin() -> iterator
{
   return iterator(m_head.next);
}

template_T_Tag inline auto intrusive_list<T,Tag>::begin() const -> const_iterator
{
   return const_iterator(m_head.next);
}

template_T_Tag inline auto intrusive_list<T,Tag>::end() -> iterator
{
   return iterator(&m_head);
}

template_T_Tag inline auto intrusive_list<T,Tag>::end() const -> const_iterator
{
   return const_iterator(&m_head);
}

template_T_Tag auto intrusive_list<T,Tag>::insert(iterator pos, T& val) -> iterator
{
   check_iterator(pos);
   hook* val_hook = &val;
   val_hook->set_list(this);
   assert(pos.h->prev->next == pos.h);
   assert(pos.h->next->prev == pos.h);
   val_hook->next = pos.h;
   val_hook->prev = pos.h->prev;
   pos.h->prev->next = val_hook;
   pos.h->prev = val_hook;
   return iterator(val_hook);
}

template_T_Tag auto intrusive_list<T,Tag>::erase(iterator pos) -> iterator
{
   check_iterator(pos);
   check_not_end(pos.h);
   pos.h->clear_list(this);
   assert(pos.h->prev->next == pos.h);
   assert(pos.h->next->prev == pos.h);
   pos.h->prev->next = pos.h->next;
   pos.h->next->prev = pos.h->prev;
   return iterator(pos.h->next);
}

template_T_Tag void intrusive_list<T,Tag>::static_erase(iterator pos)
{
   check_not_end(pos.h);
   pos.h->clear_list(nullptr);
   assert(pos.h->prev->next == pos.h);
   assert(pos.h->next->prev == pos.h);
   pos.h->prev->next = pos.h->next;
   pos.h->next->prev = pos.h->prev;
}

template_T_Tag_D inline void intrusive_list<T,Tag>::clear_and_dispose(D disposer)
{
   while (!empty())
   {
      disposer(&pop_front());
   }
}

template_T_Tag_D inline void intrusive_list<T,Tag>::pop_front_and_dispose(D disposer)
{
   disposer(&pop_front());
}

template_T_Tag_D inline void intrusive_list<T,Tag>::pop_back_and_dispose(D disposer)
{
   disposer(&pop_back());
}

template_T_Tag_D inline auto intrusive_list<T,Tag>::erase_and_dispose(iterator pos, D disposer) -> iterator
{
   T* p = from_hook(pos.h);
   iterator r = erase(pos);
   disposer(p);
   return r;
}

template_T_Tag_D inline void intrusive_list<T,Tag>::static_erase_and_dispose(iterator pos, D disposer)
{
   T* p = from_hook(pos.h);
   static_erase(pos);
   disposer(p);
}

template_T_Tag inline void intrusive_list<T,Tag>::validate() const
{
   m_head.check_list(this);
   for (hook const* h = m_head.next; h != &m_head; h = h->next)
   {
      h->check_list(this);
      assert(h->prev->next == h);
      assert(h->next->prev == h);
   }
}

template_T_Tag inline void intrusive_list<T,Tag>::validate_iterator(const_iterator i) const
{
   i.h->check_list(this);
}

#undef template_T_Tag
#undef template_T_Tag_D
