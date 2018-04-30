/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include "Common.h"

namespace bvk {

class Extensions
{
private:
   // This combination of fields is at the start of every Vulkan API structure
   struct ExtHeader
   {
      VkStructureType sType;
      void           *pNext;
   };

public:
   static VkResult EnumerateExtensionProperties(
      VkExtensionProperties *extsIn,
      uint32_t               extsInCount,
      const char            *pLayerName,
      uint32_t              *pPropertyCount,
      VkExtensionProperties *pProperties);

   // Extract the type from an extension header
   static VkStructureType GetStructureType(const void *ext)
   {
      assert(ext != nullptr);

      auto   hdr = static_cast<const ExtHeader *>(ext);
      return hdr->sType;
   }

   // Get the pNext pointer from an extension
   static void *GetNext(const void *ext)
   {
      assert(ext != nullptr);

      auto   hdr = static_cast<const ExtHeader *>(ext);
      return hdr->pNext;
   }

   template<typename T>
   static const T *FindExtensionStruct(VkStructureType findType, const void *ext)
   {
      while (ext != nullptr)
      {
         VkStructureType type = Extensions::GetStructureType(ext);
         if (type == findType)
            return static_cast<const T *>(ext);
         ext = Extensions::GetNext(ext);
      }
      return nullptr;
   }

   template<typename T>
   static T *FindExtensionStruct(VkStructureType findType, void *ext)
   {
      while (ext != nullptr)
      {
         VkStructureType type = Extensions::GetStructureType(ext);
         if (type == findType)
            return static_cast<T *>(ext);
         ext = Extensions::GetNext(ext);
      }
      return nullptr;
   }
};

} // namespace bvk
