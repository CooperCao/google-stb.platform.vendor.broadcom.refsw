/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once
#include <algorithm>
#include "vcos_types.h"

#define template_T_Tag template<typename T, typename Tag>
#define template_T_Tag_D template_T_Tag template<typename D>

// Private API
//-----------------------------------------------------------------------------

template_T_Tag inline void intrusive_slist<T,Tag>::init()
{
   m_head.next = &m_head;
   m_head.set_list(this);
   m_tail = &m_head;
}

// Assert that h is in a list but is not the special end hook (m_head).
template_T_Tag inline void intrusive_slist<T,Tag>::check_not_end(hook* h)
{
#ifndef NDEBUG
   assert(h->owner != NULL && h != &h->owner->m_head);
#endif
}

template_T_Tag inline auto intrusive_slist<T,Tag>::from_hook(hook* h) -> T*
{
   check_not_end(h);
   assume(h != nullptr);
   return static_cast<T*>(h);
}

template_T_Tag inline auto intrusive_slist<T,Tag>::get_prev(hook* h) -> hook*
{
   hook* prev = &m_head;
   while (prev->next != h)
      prev = prev->next;
   return prev;
}

// Assert that this iterator is valid for this list.
template_T_Tag inline void intrusive_slist<T,Tag>::check_iterator(const_iterator const& i)
{
   i.h->check_list(this);
}

// Public API
//-----------------------------------------------------------------------------

template_T_Tag inline intrusive_slist<T,Tag>::intrusive_slist()
{
   init();
}

template_T_Tag inline intrusive_slist<T,Tag>::intrusive_slist(intrusive_slist&& other)
{
   init();
   *this = std::move(other);
}

template_T_Tag auto intrusive_slist<T,Tag>::operator=(intrusive_slist&& other) -> intrusive_slist&
{
   assert(empty()); // can only assign to empty list

   // move pointers over, fixing up the tail pointer
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

      assert(other.m_tail->next == &other.m_head);
      m_head.next = other.m_head.next;
      m_tail = other.m_tail;
      m_tail->next = &m_head;

      // reset other
      other.m_head.next = &other.m_head;
      other.m_tail = &other.m_head;
   }
   return *this;
}

template_T_Tag inline intrusive_slist<T,Tag>::~intrusive_slist()
{
   assert(empty());
   m_head.clear_list(this);
}

template_T_Tag inline void intrusive_slist<T,Tag>::clear()
{
   // in debug, nodes need to be told they are off the list
   while (IS_DEBUG && !empty())
      pop_front();
   m_head.next = &m_head;
   m_tail = &m_head;
}

template_T_Tag void intrusive_slist<T,Tag>::swap(intrusive_slist& other)
{
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

   assert(m_tail->next == &m_head);
   assert(other.m_tail->next == &other.m_head);
   std::swap(m_head.next, other.m_head.next);
   std::swap(m_tail, other.m_tail);
   if (m_tail == &other.m_head) m_tail = &m_head;
   if (other.m_tail == &m_head) other.m_tail = &other.m_head;
   m_tail->next = &m_head;
   other.m_tail->next = &other.m_head;
}

template_T_Tag inline bool intrusive_slist<T,Tag>::empty() const
{
   assert((m_head.next == &m_head) == (m_tail == &m_head));
   return m_head.next == &m_head;
}

template_T_Tag inline size_t intrusive_slist<T,Tag>::size() const
{
   size_t size = 0;
   for (hook* h = m_head.next; h != &m_head; h = h->next, ++size);
   return size;
}

template_T_Tag inline T& intrusive_slist<T,Tag>::push_front(T& val)
{
   hook* val_hook = &val;
   val_hook->set_list(this);
   val_hook->next = m_head.next;
   m_head.next = val_hook;
   if (m_tail == &m_head)
      m_tail = val_hook;
   return val;
}

template_T_Tag inline T& intrusive_slist<T,Tag>::push_back(T& val)
{
   hook* val_hook = &val;
   val_hook->set_list(this);
   val_hook->next = &m_head;
   m_tail->next = val_hook;
   m_tail = val_hook;
   return val;
}

template_T_Tag inline T& intrusive_slist<T,Tag>::pop_front()
{
   assert(!empty());
   hook* h = m_head.next;
   T* p = from_hook(h);
   h->clear_list(this);
   m_head.next = h->next;
   if (m_head.next == &m_head)
   {
      assert(m_tail == h);
      m_tail = &m_head;
   }
   return *p;
}

template_T_Tag inline T& intrusive_slist<T,Tag>::pop_back()
{
   assert(!empty());
   hook* h = m_tail;
   T* p = from_hook(h);
   h->clear_list(this);
   m_tail = get_prev(h);
   m_tail->next = &m_head;
   return *p;
}

template_T_Tag inline T& intrusive_slist<T,Tag>::front()
{
   assert(!empty());
   return *from_hook(m_head.next);
}

template_T_Tag inline T const& intrusive_slist<T,Tag>::front() const
{
   assert(!empty());
   return *from_hook(m_head.next);
}

template_T_Tag inline auto intrusive_slist<T,Tag>::begin() -> iterator
{
   return iterator(m_head.next);
}

template_T_Tag inline auto intrusive_slist<T,Tag>::begin() const -> const_iterator
{
   return const_iterator(m_head.next);
}

template_T_Tag inline auto intrusive_slist<T,Tag>::end() -> iterator
{
   return iterator(&m_head);
}

template_T_Tag inline auto intrusive_slist<T,Tag>::end() const -> const_iterator
{
   return const_iterator(&m_head);
}

template_T_Tag auto intrusive_slist<T,Tag>::insert(iterator pos, T& val) -> iterator
{
   check_iterator(pos);
   hook* pos_prev = get_prev(pos.h);
   hook* val_hook = &val;
   val_hook->set_list(this);
   val_hook->next = pos_prev->next;
   pos_prev->next = val_hook;
   if (pos.h == &m_head)
      m_tail = val_hook;
   return iterator(val_hook);
}

template_T_Tag auto intrusive_slist<T,Tag>::erase(iterator pos) -> iterator
{
   check_iterator(pos);
   check_not_end(pos.h);
   hook* pos_prev = get_prev(pos.h);
   pos.h->clear_list(this);
   pos_prev->next = pos.h->next;
   if (m_tail == pos.h)
      m_tail = pos_prev;
   return iterator(pos_prev->next);
}

template_T_Tag inline auto intrusive_slist<T,Tag>::insert_after(iterator pos, T& val) -> iterator
{
   check_iterator(pos);
   check_not_end(pos.h);
   hook* val_hook = &val;
   val_hook->set_list(this);
   val_hook->next = pos.h->next;
   pos.h->next = val_hook;
   if (m_tail == pos.h)
      m_tail = val_hook;
   return iterator(val_hook);
}

template_T_Tag inline auto intrusive_slist<T,Tag>::erase_after(iterator pos) -> iterator
{
   check_iterator(pos);
   check_not_end(pos.h);
   hook* erase_hook = pos.h->next;
   check_not_end(erase_hook);
   erase_hook->clear_list(this);
   pos.h->next = erase_hook->next;
   if (m_tail == erase_hook)
      m_tail = pos.h;
   return iterator(pos.h->next);
}

template_T_Tag_D inline void intrusive_slist<T,Tag>::clear_and_dispose(D disposer)
{
   while (!empty())
   {
      disposer(&pop_front());
   }
}

template_T_Tag_D inline void intrusive_slist<T,Tag>::pop_front_and_dispose(D disposer)
{
   disposer(&pop_front());
}

template_T_Tag_D inline void intrusive_slist<T,Tag>::pop_back_and_dispose(D disposer)
{
   disposer(&pop_back());
}

template_T_Tag_D inline auto intrusive_slist<T,Tag>::erase_and_dispose(iterator pos, D disposer) -> iterator
{
   T* p = from_hook(pos.h);
   iterator r = erase(pos);
   disposer(p);
   return r;
}

template_T_Tag_D inline auto intrusive_slist<T,Tag>::erase_after_and_dispose(iterator pos, D disposer) -> iterator
{
   T* p = from_hook(pos.h->next);
   iterator r = erase_after(pos);
   disposer(p);
   return r;
}

template_T_Tag inline void intrusive_slist<T,Tag>::validate() const
{
   m_head.check_list(this);
   hook const* last_hook = &m_head;
   for (hook const* h = m_head.next; h != &m_head; h = h->next)
   {
      h->check_list(this);
      last_hook = h;
   }
   assert(m_tail == last_hook);
   assert(m_tail->next == &m_head);
}

#undef template_T_Tag
#undef template_T_Tag_D
