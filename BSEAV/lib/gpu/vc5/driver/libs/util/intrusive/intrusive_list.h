/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once
#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include "libs/util/assert_helpers.h"

// forward declare types
struct intrusive_list_default_tag {};
template<typename T, typename Tag = intrusive_list_default_tag> class intrusive_list;
template<typename T, typename Tag = intrusive_list_default_tag> class intrusive_list_hook;

//! A derivation hook for adding elements to a doubly-linked intrusive list.
template<typename T, typename Tag>
class intrusive_list_hook
{
protected:
   typedef intrusive_list<T,Tag> list;

   intrusive_list_hook()
   {
     #ifndef NDEBUG
      owner = NULL;
     #endif
   }

   ~intrusive_list_hook()
   {
     #ifndef NDEBUG
      assert(owner == NULL);
     #endif
   }

private:
   friend class intrusive_list<T,Tag>;

   // copy not allowed
   intrusive_list_hook(intrusive_list_hook const&);
   intrusive_list_hook& operator=(intrusive_list_hook const&);

  #ifndef NDEBUG
   void set_list(list const* l) { assert(owner == NULL); owner = l; }            // assert that not already on a list
   void clear_list(list const* l) { assert(!l || owner == l); owner = NULL; }          // assert that on the list being removed from
   void check_list(list const* l) const { assert(owner == l); }                  // assert on the expected list.
  #else
   void set_list(list const*) {}
   void clear_list(list const*) {}
   bool check_list(list const*) const { return true; }
  #endif

private:
   intrusive_list_hook<T,Tag>* next;
   intrusive_list_hook<T,Tag>* prev;
  #ifndef NDEBUG
   list const* owner;
  #endif
};

//! A doubly-linked intrusive list.
template<typename T, typename Tag>
class intrusive_list
{
public:
   // public types
   typedef T value_type;
   typedef T* pointer;
   typedef T& reference;
   typedef T const& const_reference;
   typedef size_t size_type;
   typedef ptrdiff_t difference_type;
   typedef intrusive_list_hook<T,Tag> hook;
   class iterator;
   class const_iterator;

   //! Creates an empty list.
   intrusive_list();

   //! Move constructor from another list.
   intrusive_list(intrusive_list&& other);

   //! Move assignment from another list. This list must be empty.
   intrusive_list& operator=(intrusive_list&& other);

   //! The destructor. The list must be empty.
   ~intrusive_list();

   //! O(1). Erases all of the elements.
   void clear();

   //! O(1). Swaps the contents of two lists.
   void swap(intrusive_list& other);

   //! O(1). Returns true if the list's size is 0.
   bool empty() const;

   //! O(N). Returns the number of elements in the list.
   size_t size() const;

   //! O(1). Inserts a new element at the beginning. Returns a reference to the element.
   T& push_front(T& val);

   //! O(1). Inserts a new element at the end. Returns a reference to the element.
   T& push_back(T& val);

   //! O(1). Removes the first element.
   T& pop_front();

   //! O(1). Removes the last element.
   T& pop_back();

   //! O(1). Returns the first element.
   T& front();

   //! O(1). Returns the first element.
   T const& front() const;

   //! O(1). Returns an iterator pointing to the beginning of the list.
   iterator begin();

   //! O(1). Returns an iterator pointing to the beginning of the list.
   const_iterator begin() const;

   //! O(1). Returns an iterator pointing to the end of the list.
   iterator end();

   //! O(1). Returns an iterator pointing to the end of the list.
   const_iterator end() const;

   //! O(1). Inserts val before pos. Returns an iterator pointing to val.
   iterator insert(iterator pos, T& val);

   //! O(1). Erases the element at pos. Returns an iterator pointing to the next element.
   iterator erase(iterator pos);

   //! O(1). Erases the element at pos from the list it is on.
   static void static_erase(iterator pos);

   //! O(N). Erases all of the elements invoking the disposer on each pointer.
   template<typename D> void clear_and_dispose(D disposer);

   //! O(1). Removes the first element and invokes the disposer on it.
   template<typename D> void pop_front_and_dispose(D disposer);

   //! O(1). Removes the last element and invokes the disposer on it.
   template<typename D> void pop_back_and_dispose(D disposer);

   //! O(1). Removes the element at pos and invokes the disposer on it.
   template<typename D> iterator erase_and_dispose(iterator pos, D disposer);

   //! O(1). Removes the element at pos from the list it is on and invokes the disposer on it.
   template<typename D> static void static_erase_and_dispose(iterator pos, D disposer);

   //! O(N). Validate the integrity of the list. (debugging only).
   void validate() const;

   //! O(1). Validate an iterator is in this list.
   void validate_iterator(const_iterator i) const;

   class const_iterator
   {
   public:
      const_iterator() {};
      const_iterator(hook const* h) : h(const_cast<hook*>(h)) {}

      const_iterator operator++(int) { check_not_end(h); const_iterator r = *this; h = h->next; return r; }
      const_iterator& operator++() { check_not_end(h); h = h->next; return *this; }
      const_iterator operator--(int) { check_not_begin(h); const_iterator r = *this; h = h->prev; return r; }
      const_iterator& operator--() { check_not_begin(h); h = h->prev; return *this; }

      T const* operator->() const { return from_hook(h); }
      T const& operator*() const { return *from_hook(h); }

      friend bool operator==(const_iterator const& a, const_iterator const& b) { return a.h == b.h; }
      friend bool operator!=(const_iterator const& a, const_iterator const& b) { return a.h != b.h; }

   private:
      friend class intrusive_list;
      friend class iterator;

      hook* h;
   };

   class iterator : public const_iterator
   {
   public:
      iterator() {}
      iterator(hook* h) : const_iterator(h) {}

      iterator operator++(int) { check_not_end(h); iterator r = *this; h = h->next; return r; }
      iterator& operator++() { check_not_end(h); h = h->next; return *this; }
      iterator operator--(int) { check_not_begin(h); iterator r = *this; h = h->prev; return r; }
      iterator& operator--() { check_not_begin(h); h = h->prev; return *this; }

      T* operator->() const { return from_hook(h); }
      T& operator*() const { return *from_hook(h); }

   private:
      friend class intrusive_list;

      using const_iterator::h;
   };

private:
   // copy not allowed
   intrusive_list(intrusive_list const& other);
   intrusive_list& operator=(intrusive_list const& other);

   void init();
   static void check_not_begin(hook* h); // Assert that h is in a list but is not the first in the list
   static void check_not_end(hook* h); // Assert that h is in a list but is not the special end hook (m_head)
   static T* from_hook(hook* h);
   void check_iterator(const_iterator const& i);

private:
   hook m_head;
};

#include "intrusive_list.inl"
