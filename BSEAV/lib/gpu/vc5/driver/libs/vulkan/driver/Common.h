/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include <cstdint>
#include <cstdlib>
#include <cstddef>
#include <cstring>
#include <type_traits>
#include <iostream>
#include <string.h>

#include "vulkan.h"
#include "Dispatchable.h"
#include "NonCopyable.h"
#include "APIVersion.h"
#include "Bitfield.h"

#include "libs/core/v3d/v3d_gen.h"
#include "libs/util/common.h"

namespace bvk
{

using JobID = uint64_t;

// This pattern appears all over the Vulkan code.
// Might as well make it common.
template <typename T>
VkResult sizedListQuery(T *list, uint32_t *size, const T *actualList, uint32_t actualSize)
{
   VkResult result = VK_SUCCESS;

   if (list == nullptr)
      *size = actualSize;
   else
   {
      if (actualSize > *size)
         result = VK_INCOMPLETE;

      *size = std::min(actualSize, *size);
      memcpy(list, actualList, *size * sizeof(T));
   }

   return result;
}

class PhysicalDevice;

class PhysicalDeviceDispatchableWrapper : public NonCopyable, public Dispatchable
{
public:
   PhysicalDeviceDispatchableWrapper() = default;  // This does not acquire the PhysicalDevice
   ~PhysicalDeviceDispatchableWrapper();

   void AcquireDevice();
   PhysicalDevice *GetPhysicalDevice() { return m_physicalDevice; }

private:
   PhysicalDevice *m_physicalDevice = nullptr;
};

// If T is not Dispatchable, converts nonDispatchableObj of type T * to handle of type H
template <typename H, typename T>
inline typename std::enable_if<!std::is_base_of<Dispatchable, T>::value, H>::type
toHandle(T *nonDispatchableObj)
{
   return reinterpret_cast<H>(nonDispatchableObj);
}

// If T is Dispatchable, converts dispatchableObj of type T * to handle of type H
template <typename H, typename T>
inline typename std::enable_if<std::is_base_of<Dispatchable, T>::value, H>::type
toHandle(T *dispatchableObj)
{
   return reinterpret_cast<H>(dispatchableObj->GetDispatchTable());
}

template <typename H>
inline H
toHandle(PhysicalDeviceDispatchableWrapper *dispatchableObj)
{
   return reinterpret_cast<H>(dispatchableObj->GetDispatchTable());
}

// Don't allow toHandle to be used for PhysicalDevice. A PhysicalDevice
// has to be wrapped in a PhysicalDeviceDispatchableWrapper before being
// used as a handle, so we can't use this.
template <typename H>
inline H
toHandle(PhysicalDevice *dispatchableObj) = delete;

// If R is not Dispatchable, converts handle of type H to an R *
template <typename R, typename H>
inline typename std::enable_if<!std::is_base_of<Dispatchable, R>::value, R*>::type
fromHandle(H handle)
{
   return reinterpret_cast<R*>(handle);
}

// If R is Dispatchable, converts handle of type H to an R *
template <typename R, typename H>
inline typename std::enable_if<std::is_base_of<Dispatchable, R>::value, R*>::type
fromHandle(H handle)
{
   return Dispatchable::FromDispatchTable<R>(handle);
}

// Specialized template for Instance (as VkInstance can be null)
template <typename T>
inline T *
fromHandle(VkInstance handle)
{
   return handle != nullptr ? Dispatchable::FromDispatchTable<T>(handle) : nullptr;
}

// Specialized template for PhysicalDevice which is really a singleton, but as it's
// a dispatchable object, the handle has to be unique. PhysicalDeviceDispatchableWrapper
// is a dispatchable object that wraps the singleton pointer.
template <typename T>
inline T *
fromHandle(VkPhysicalDevice handle)
{
   auto *wrapper = Dispatchable::FromDispatchTable<PhysicalDeviceDispatchableWrapper>(handle);
   return wrapper->GetPhysicalDevice();
}

// Exception class to throw on OODM
class bad_device_alloc {};

class nothing_to_do {};

void Intersect(VkRect2D *result, const VkRect2D &r1, const VkRect2D &r2);
void Union(VkRect2D *result, const VkRect2D &r1, const VkRect2D &r2);

// Helper for debug log messages to save some pointless typing
inline const char *TF(bool b) noexcept { return b ? "true" : "false"; }

#define NOT_IMPLEMENTED_YET { std::cerr << "Not yet implemented : " << __FUNCTION__ << " Line: " << __LINE__ << std::endl; }

} // namespace bvk
