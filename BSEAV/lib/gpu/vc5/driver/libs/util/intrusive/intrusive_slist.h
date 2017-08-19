/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once
#include <stdint.h>
#include "libs/util/assert_helpers.h"

// forward declare types
struct intrusive_slist_default_tag {};
template<typename T, typename Tag = intrusive_slist_default_tag> class intrusive_slist;
template<typename T, typename Tag = intrusive_slist_default_tag> class intrusive_slist_hook;

//! A derivation hook for adding elements to a singly-linked intrusive list.
template<typename T, typename Tag>
class intrusive_slist_hook
{
protected:
   typedef intrusive_slist<T,Tag> list;

   intrusive_slist_hook()
   {
     #ifndef NDEBUG
      owner = NULL;
     #endif
   }

   ~intrusive_slist_hook()
   {
     #ifndef NDEBUG
      assert(owner == NULL);
     #endif
   }

private:
   friend class intrusive_slist<T,Tag>;

   // copy not allowed
   intrusive_slist_hook(intrusive_slist_hook const&);
   intrusive_slist_hook& operator=(intrusive_slist_hook const&);

  #ifndef NDEBUG
   void set_list(list const* l) { assert(owner == NULL); owner = l; }            // assert that not already on a list
   void clear_list(list const* l) { assert(owner == l); owner = NULL; }          // assert that on the list being removed from
   void check_list(list const* l) const { assert(owner == l); }                  // assert on the expected list.
  #else
   void set_list(list const*) {}
   void clear_list(list const*) {}
   bool check_list(list const*) const { return true; }
  #endif

private:
   intrusive_slist_hook<T,Tag>* next;
  #ifndef NDEBUG
   list const* owner;
  #endif
};

//! A singly-linked intrusive list.
template<typename T, typename Tag>
class intrusive_slist
{
public:
   // public types
   typedef T value_type;
   typedef T* pointer;
   typedef T& reference;
   typedef T const& const_reference;
   typedef size_t size_type;
   typedef ptrdiff_t difference_type;
   typedef intrusive_slist_hook<T,Tag> hook;
   class iterator;
   class const_iterator;

   //! Creates an empty list.
   intrusive_slist();

   //! Move constructor from another list.
   intrusive_slist(intrusive_slist&& other);

   //! Move assignment from another list. This list must be empty.
   intrusive_slist& operator=(intrusive_slist&& other);

   //! The destructor. The list must be empty.
   ~intrusive_slist();

   //! O(1). Erases all of the elements.
   void clear();

   //! O(1). Swaps the contents of two lists.
   void swap(intrusive_slist& other);

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

   //! O(N). Removes the last element.
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

   //! O(N). Inserts val before pos. Returns an iterator pointing to val.
   iterator insert(iterator pos, T& val);

   //! O(1). Inserts val after pos. Returns an iterator pointing to val.
   iterator insert_after(iterator pos, T& val);

   //! O(N). Erases the element at pos. Returns an iterator pointing to the next element.
   iterator erase(iterator pos);

   //! O(1). Erases the element after pos. Returns an iterator pointing to the next element.
   iterator erase_after(iterator pos);

   //! O(N). Erases all of the elements invoking the disposer on each pointer.
   template<typename D> void clear_and_dispose(D disposer);

   //! O(1). Removes the first element and invokes the disposer on it.
   template<typename D> void pop_front_and_dispose(D disposer);

   //! O(N). Removes the last element and invokes the disposer on it.
   template<typename D> void pop_back_and_dispose(D disposer);

   //! O(N). Removes the element at pos and invokes the disposer on it.
   template<typename D> iterator erase_and_dispose(iterator pos, D disposer);

   //! O(1). Removes the element after pos and invokes the disposer on it.
   template<typename D> iterator erase_after_and_dispose(iterator pos, D disposer);

   //! O(N). Validate the integrity of the list. (debugging only).
   void validate() const;

   class const_iterator
   {
   public:
      const_iterator() {};
      const_iterator(hook const* h) : h(const_cast<hook*>(h)) {}

      const_iterator operator++(int) { check_not_end(h); const_iterator r = *this; h = h->next; return r; }
      const_iterator& operator++() { check_not_end(h); h = h->next; return *this; }

      T const* operator->() const { return from_hook(h); }
      T const& operator*() const { return *from_hook(h); }

      friend bool operator==(const_iterator const& a, const_iterator const& b) { return a.h == b.h; }
      friend bool operator!=(const_iterator const& a, const_iterator const& b) { return a.h != b.h; }

   private:
      friend class intrusive_slist;
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

      T* operator->() const { return from_hook(h); }
      T& operator*() const { return *from_hook(h); }

   private:
      friend class intrusive_slist;

      using const_iterator::h;
   };

private:
   // copy not allowed
   intrusive_slist(intrusive_slist const& other);
   intrusive_slist& operator=(intrusive_slist const& other);

   void init();
   static void check_not_end(hook* h); // Assert that h is in a list but is not the special end hook (m_head)
   static T* from_hook(hook* h);
   hook* get_prev(hook* h);
   void check_iterator(const_iterator const& i);

private:
   hook m_head;
   hook* m_tail;
};

#include "intrusive_slist.inl"
