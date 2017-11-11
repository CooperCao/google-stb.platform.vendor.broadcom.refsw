/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

namespace bvk
{

// Abstract base class that enables an object to be 'dispatchable'.
class Dispatchable
{
public:
   static constexpr uintptr_t LoaderMagic = 0x01CDC0DE;

   typedef union
   {
      uintptr_t loaderMagic;
      void     *loaderData;
   } VK_LOADER_DATA;

   Dispatchable()
   {
      m_dispatchTable.loaderMagic = LoaderMagic;
   }

   // The dispatch table pointer will be what the outside world uses as the
   // VkInstance handle. Give access to it.
   void *GetDispatchTable() const
   {
      return const_cast<void*>(reinterpret_cast<const void*>(&m_dispatchTable));
   }

   // Convert from a dispatch table pointer (the Vulkan object handle) back to the
   // real C++ object instance pointer.
   template <class T>
   static T *FromDispatchTable(void *dt)
   {
      T        *obj    = static_cast<T*>(0);
      uintptr_t offset = reinterpret_cast<uintptr_t>(&obj->m_dispatchTable);
      return reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(dt) - offset);
   }

private:
   VK_LOADER_DATA m_dispatchTable;
};

} // namespace bvk
