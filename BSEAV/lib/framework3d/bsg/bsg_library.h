/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __BSG_LIBRARY_H__
#define __BSG_LIBRARY_H__

#include "bsg_common.h"
#include "bsg_exception.h"

#include <stdio.h>
#include <stdint.h>
#include <vector>
#include <list>
#include <string>
#include <map>
#include <sstream>

namespace bsg
{

// @cond
class Libraries
{
public:
   static void Create();
   static void Destroy();
};

class RefCount
{
public:
   RefCount() :
      m_refCount(0)
   {
   }

   void IncRefCount()
   {
      m_refCount += 1;
   }

   uint32_t DecRefCount()
   {
      m_refCount -= 1;
      return m_refCount;
   }

private:
   uint32_t m_refCount;
};
// @endcond

/** @addtogroup handles Handles

<B>General Information</B>

Many of the BSG classes (bsg::SceneNode, bsg::Material, bsg::Effect etc.) are managed.
Applications do not usually manipulate these objects directly.  Rather, they interact with them via "handles".  A handle looks
like a pointer in application code, but the ownership of the underlying object remains with BSG.

BSG maintains reference counts for all the managed objects.  When the reference count drops to zero, the
managed object is deleted.  This mechanism simplifies the creation, deletion and editing of scene-graphs, leaving
applications to worry about the logic of the graph and not about the management of resources.

To create e.g. a scene node handle, simply declare a handle thus:
\code
SceneNodeHandle   handle;
\endcode

This declares a scene node handle and initialises it to a zero/null pointer.  To bind the handle to a new object:

\code
SceneNodeHandle handle(New);
\endcode

Handles can be freely assigned and copied:

\code
MaterialHandle h0;
MaterialHandle h1(New);
MaterialHandle h2(h1);

if (h0.IsNull())
   h0 = h1;
\endcode

Often you will want to declare handles as members of your application classes and populate them in the constructors.

As the handles are managed by BSG and they can easily be shared.  For example, a scene graph can use the same
bsg::Material in the multiple places, or indeed can have the same scene node appear in multiple places.

NB: that if you hold handles inside objects which are created on the heap via e.g. new, the handles will
persist and hence their managed objects will also persist until the heap object is deleted.

Handles can optionally be associated with a name and an integer tag.  Names must be unique, but tags can be shared.
If the application doesn't supply a name, a default name will be generated from a name supply.  The default tag is zero.
\code
SceneNodeHandle handle1(New, "myNode");
SceneNodeHandle handle2(New, 23);
SceneNodeHandle handle3(New, "myOtherNode", 6);
\endcode

Names are useful as unique identifiers for handles.  Tags can be useful for grouping. e.g. a tag could be used to arrange
handles into several sets which could be treated differently to one another.

<B>Advanced Note on Handle Traits</B>

Every client class hierarchy of the library framework defines a traits class containing
typedefs for the following items:

- Base: the class at the base of the hierarchy (e.g. SceneNode).  This is the type that will be used
        to store items in the library.
- Derived: the class for which the new handle type is being defined
- BaseTraits: the traits class for the base type (e.g. SceneNodeTraits)

E.g.
Traits for a class derived from SceneNode would be:
\code
using namespace bsg;

struct SceneNodeDerivedTraits
{
  typedef SceneNode         Base;
  typedef SceneNodeDerived  Derived;
  typedef SceneNodeTraits   BaseTraits;
};

typedef Handle<SceneNodeDerivedTraits>   SceneNodeDerivedHandle;
\endcode

Objects referenced by this new handle type can be treated like a SceneNodeHandle for storage in generic containers.
@{
*/

template <class T>
class Library;

//! This is a syntactic class used to provide auto-allocating overloads for the handle
//! constructor classes.
//! Pass the global value "New" of this type to Handle constructors to automatically create the handled object.
class AllocNew
{
};

//! The one and only value of type AllocNew called bsg::New -- used as argument to handle constructors when allocating an object.
extern AllocNew  New;

///////////////////////////////////////////////////////////////////////////////////////////////////

// @cond
template <class T>
class ItemInfo
{
public:
   void Clear()
   {
      m_ptr  = 0;
      m_name = "";
      m_tag  = 0;
   }

   ItemInfo()
   {
      Clear();
   }

   ItemInfo(T *ptr, const std::string &name, uint32_t tag) :
      m_ptr(ptr),
      m_name(name),
      m_tag(tag)
   {}

   T *GetPtr()                         { return m_ptr;   }
   const T *GetPtr() const             { return m_ptr;   }
   const std::string &GetName() const  { return m_name;  }
   uint32_t          GetTag() const    { return m_tag;   }

private:
   T           *m_ptr;
   std::string m_name;
   uint32_t    m_tag;
};
// @endcond

//! Handle
//!
//! The Handle template class implements handles for the type described in the traits parameter.
//! Applications normally use instances of this class via a typedef e.g. bsg::SceneNodeHandle is really a Handle<SceneNodeTraits>
template <class Traits>
class Handle
{
   typedef  typename Traits::BaseTraits   BaseTraits;
   typedef  typename Traits::Base         Base;
   typedef  typename Traits::Derived      Derived;
   typedef  Library<BaseTraits>           BaseLibrary;

   friend class Library<BaseTraits>;

public:
   // @cond
   enum
   {
      BAD_INDEX = -1
   };
   // @endcond

   //! Construct a "null" handle
   Handle();

   //! Construct a "new" handle -- i.e. one that points to a freshly allocated object.  Pass optional tag as second argument. Pass the value bsg::New as the first argument.
   Handle(const AllocNew &, uint32_t tag = 0);

   //! Construct a "new" handle -- i.e. one that points to a freshly allocated object.  Pass unique name as second argument and optional tag as third. Pass the value bsg::New as the first argument.
   Handle(const AllocNew &, const std::string &name, uint32_t tag = 0);

   //! Copy construct a handle
   Handle(const Handle &handle);

   //! Construct a handle from an existing entry in the library.  The handled objects will be searched for the name, so if there
   //! are a lot, this could be costly.  It is often better for an application to manage its own mappings.
   Handle(const std::string &name);

   //! Assign a handle
   Handle &operator=(const Handle &handle);

   //! Dispose of a handle and if ref count reaches zero, then delete the managed object
   ~Handle();

   //! Delete the underlying object (this is dangerous as it will orphan handles)
   void Delete();

   //! Set pointer to null and decrement the reference count of the previously referred object (if any)
   void Clear();

   //! Check if the pointer is valid
   bool  IsNull() const;

   //! Get (base) pointer for this handle.  Use with extreme caution.  Applications should rarely need
   //! to be aware of the pointer and will get into trouble if the object is deleted.
   Base  *GetPtr() const;

   //! Get the handle's name
   const std::string &GetName() const;

   //! Get the handle's tag
   uint32_t          GetTag() const;

   //! Get the derived pointer for the managed object.  See comments on GetPtr().
   Derived *Ptr() const;

   //! The * and -> operators return pointers/references to the derived types so that
   //! derived classes behave as expected.  Use as handle->Method() or (*handle).Method().
   //! @{
   Derived *operator->() const;
   Derived &operator*() const;
   //! @}

   //! Cast the derived handle type into the base handle type.
   //! All we are doing is re-interpreting an index to a derived
   //! class as an index to a base class.  This is OK so long
   //! as the traits have not lied about the type relationship.
   operator Handle<BaseTraits>()
   {
      return *reinterpret_cast<Handle<BaseTraits> *>(this);
   }

   //! operator< can be used for sorting by the internal id
   bool operator<(const Handle &rhs)  const;

   //! Compare handles for equality
   bool operator==(const Handle &rhs) const;

   //! Compare handles for inequality
   bool operator!=(const Handle &rhs) const;

private:
   // @cond
   // Only to be used by Library
   Handle(uint32_t index);

   void  DecRefCount() const;
   void  IncRefCount() const;

private:
   uint32_t       m_index;

   //! @endcond

#ifndef NDEBUG
   //! Convenience for debugging purposes only -- cache the pointer to which this handle refers
   Base  *m_debugPtr;
#endif
};

// @cond
template <class Traits>
class Library
{
   friend class Libraries;

public:
   typedef Handle<Traits>            HandleType;
   typedef typename Traits::Base        Base;
   typedef typename Traits::BaseTraits  BaseTraits;
   typedef Library<BaseTraits>          BaseLibrary;

   // Puts the pointer in the library -- used by handles during construction, doesn't increment ref counts,
   // that's the handle's job.
   uint32_t OwnNewItem(Base *elem, const std::string &name, uint32_t tag, bool checkName);

   // Removes the entry corresponding to index, doesn't delete the managed pointer (the handle does this job)
   void RemoveEntry(uint32_t index);

   // Find a handle's index
   uint32_t FindIndex(const std::string &name);

   // Retrieve info
   ItemInfo<Base> &GetInfo(uint32_t index);

private:
   // Used by Libraries class
   Library();

   // Static methods to create and destroy the singleton libraries
   static void CreateLibrary();
   static void DestroyLibrary();

private:
   std::vector<ItemInfo<Base> >     m_elements;
   uint32_t                         m_lastElement;
   std::list<uint32_t>              m_freeList;

   std::map<std::string, uint32_t>  m_nameToIndex;

public:
   static Library                   *m_library;
};

#ifdef __clang__
// Doesn't build in clang without this declaration.
// gcc 4.8.4 doesn't like it though
template <typename Traits> Library<Traits> *Library<Traits>::m_library;
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////

//! Check if the pointer is valid
template <class T>
bool Handle<T>::IsNull() const
{
   return m_index == BAD_INDEX;
}

template <class T>
typename Handle<T>::Base *Handle<T>::GetPtr() const
{
   Base  *ptr = BaseLibrary::m_library->GetInfo(m_index).GetPtr();

   return ptr;
}

// Decrement ref count and delete if necessary
template <class T>
void Handle<T>::DecRefCount() const
{
   BaseLibrary *lib = BaseLibrary::m_library;

   if (!IsNull())
   {
      if (lib != 0)
      {
         Base  *ptr = GetPtr();

         if (ptr != 0 && ptr->DecRefCount() == 0)
         {
            delete ptr;
            lib->RemoveEntry(m_index);
         }
      }
   }
}

template <class T>
void Handle<T>::IncRefCount() const
{
   if (!IsNull())
   {
      Base  *ptr = GetPtr();

      if (ptr != 0)
         ptr->IncRefCount();
   }
}

// Delete referenced object (dangerous as it will orphan existing handles)
template <class T>
void Handle<T>::Delete()
{
   BaseLibrary *lib = BaseLibrary::m_library;

   if (!IsNull())
   {
      if (lib != 0)
      {
         Base  *ptr = GetPtr();

         if (ptr != NULL)
         {
            while (ptr != 0 && ptr->DecRefCount() > 0)
               continue;

            delete ptr;
            lib->RemoveEntry(m_index);
         }
      }
   }

   m_index = BAD_INDEX;

#ifndef NDEBUG
   m_debugPtr = 0;
#endif
}


// Reset handle to null
template <class T>
void Handle<T>::Clear()
{
   DecRefCount();

   m_index = BAD_INDEX;

#ifndef NDEBUG
   m_debugPtr = 0;
#endif
}

// A Null handle
template <class T>
Handle<T>::Handle() :
   m_index(BAD_INDEX)
{
#ifndef NDEBUG
   m_debugPtr = 0;
#endif
}

// An auto-created handle
template <class T>
Handle<T>::Handle(const AllocNew &, uint32_t tag) :
   m_index(BaseLibrary::m_library->OwnNewItem(new Derived, "", tag, false))
{
   IncRefCount();

#ifndef NDEBUG
   m_debugPtr = m_index != BAD_INDEX ? GetPtr() : 0;
#endif
}

// An auto-created handle with a name
template <class T>
Handle<T>::Handle(const AllocNew &, const std::string &name, uint32_t tag) :
   m_index(BaseLibrary::m_library->OwnNewItem(new Derived, name, tag, true))
{
   IncRefCount();

#ifndef NDEBUG
   m_debugPtr = m_index != BAD_INDEX ? GetPtr() : 0;
#endif
}

// Copy construct
template <class T>
Handle<T>::Handle(const Handle<T> &handle) :
   m_index(handle.m_index)
{
   IncRefCount();

#ifndef NDEBUG
   m_debugPtr = m_index != BAD_INDEX ? GetPtr() : 0;
#endif
}

template <class T>
Handle<T>::Handle(uint32_t index) :
   m_index(index)
{
   IncRefCount();

#ifndef NDEBUG
   m_debugPtr = m_index != BAD_INDEX ? GetPtr() : 0;
#endif
}

template <class T>
Handle<T>::Handle(const std::string &name) :
   m_index(Library<BaseTraits>::m_library->FindIndex(name))
{
   IncRefCount();

#ifndef NDEBUG
   m_debugPtr = m_index != BAD_INDEX ? GetPtr() : 0;
#endif
}

// Delete the handle.
template <class T>
Handle<T>::~Handle()
{
   DecRefCount();
}

// Assignment
template <class T>
Handle<T> &Handle<T>::operator=(const Handle<T> &handle)
{
   // Check for self assignment
   if (&handle != this)
   {
      DecRefCount();

      m_index = handle.m_index;

      IncRefCount();

#ifndef NDEBUG
   m_debugPtr = m_index != BAD_INDEX ? GetPtr() : 0;
#endif
   }

   return *this;
}

// Get the name associated with this handle
template <class T>
const std::string &Handle<T>::GetName() const
{
   return BaseLibrary::m_library->GetInfo(m_index).GetName();
}

// Get the tag associated with this handle
template <class T>
uint32_t Handle<T>::GetTag() const
{
   return BaseLibrary::m_library->GetInfo(m_index).GetTag();
}

template <class T>
typename Handle<T>::Derived *Handle<T>::Ptr() const
{
   return static_cast<Derived *>(GetPtr());
}

template <class T>
typename Handle<T>::Derived *Handle<T>::operator->() const
{
   return Ptr();
}

template <class T>
typename Handle<T>::Derived &Handle<T>::operator*() const
{
   return *Ptr();
}

//! operator< can be used for sorting by id
template <class T>
bool Handle<T>::operator<(const Handle<T> &rhs) const
{
   return m_index < rhs.m_index;
}

template <class T>
bool Handle<T>::operator==(const Handle<T> &rhs) const
{
   return m_index == rhs.m_index;
}

template <class T>
bool Handle<T>::operator!=(const Handle<T> &rhs) const
{
   return m_index != rhs.m_index;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

template <class T>
Library<T>::Library() :
   m_elements(10),
   m_lastElement(0)
{
}

template <class T>
uint32_t Library<T>::OwnNewItem(Base *elem, const std::string &name, uint32_t tag, bool checkName)
{
   uint32_t slot = m_lastElement;

   // Auto generated names do not need to be checked
   if (checkName)
   {
      if (m_nameToIndex.find(name) != m_nameToIndex.end())
         BSG_THROW("Duplicate name (" << name << ") -- library objects must have unique names");
   }

   if (m_freeList.size() > 0)
   {
      slot = m_freeList.front();
      m_freeList.pop_front();
   }
   else
   {
      m_lastElement++;
   }

   if (slot >= m_elements.size())
   {
      m_elements.resize((slot + 1) * 3 / 2);    // Grow the list 1.5 times bigger
   }

   m_elements[slot] = ItemInfo<Base>(elem, name, tag);

   if (name != "")
      m_nameToIndex[name] = slot;

   return slot;
}

template <class T>
void Library<T>::RemoveEntry(uint32_t index)
{
   if (index < m_elements.size())
   {
      ItemInfo<Base> &info = m_elements[index];

      if (info.GetPtr() != 0)
      {
         m_nameToIndex.erase(info.GetName());

         info.Clear();

         m_freeList.push_back(index);
      }
   }
}

//! Create the library
template <class T>
void Library<T>::CreateLibrary()
{
   if (m_library != 0)
      delete m_library;
   m_library = new Library;
}

//! Destroy the library
template <class T>
void Library<T>::DestroyLibrary()
{
   if (m_library != 0)
   {
      for (uint32_t i = 0; i < m_library->m_elements.size(); ++i)
      {
         ItemInfo<Base> &info = m_library->m_elements[i];
         Base           *ptr  = info.GetPtr();

         if (ptr != 0)
         {
            // This is a "leaking" pointer, so tidy it up
            // Can be caused by statics holding on to handles
            delete ptr;
         }
      }

      delete m_library;
   }

   m_library = 0;
}

template <class T>
uint32_t Library<T>::FindIndex(const std::string &name)
{
   std::map<std::string, uint32_t>::iterator iter = m_nameToIndex.find(name);

   if (iter == m_nameToIndex.end())
      return Handle<T>::BAD_INDEX;

   return iter->second;
}

template <class T>
ItemInfo<typename Library<T>::Base> &Library<T>::GetInfo(uint32_t index)
{
#ifndef NDEBUG
   if (index >= m_elements.size())
      BSG_THROW("Index out of range");
#endif

   return m_elements[index];
}

// @endcond

//! @}

}

#endif /* __BSG_LIBRARY_H__ */
