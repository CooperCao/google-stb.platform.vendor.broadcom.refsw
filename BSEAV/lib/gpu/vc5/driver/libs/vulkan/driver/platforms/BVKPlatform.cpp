/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#if EMBEDDED_SETTOP_BOX

#include <stdexcept>

#include "BVKPlatform.h"

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
#include "sched_android.h"
#else
#include "sched_nexus.h"
#endif

#include "memory_drm.h"

extern "C" {
extern void BVK_RegisterMemoryInterface(BEGL_MemoryInterface *iface);
extern void BVK_RegisterSchedInterface(BEGL_SchedInterface *iface);
}

namespace bvk
{

void BVKPlatform::Register()
{
   m_memoryInterface = CreateDRMMemoryInterface();
   if (m_memoryInterface == nullptr)
      throw std::runtime_error("unable to create the platform DRM memory interfaces");

#ifdef VK_USE_PLATFORM_ANDROID_KHR
   m_schedInterface = CreateAndroidSchedInterface(m_memoryInterface);
#else
   m_schedInterface  = CreateSchedInterface(m_memoryInterface, NULL);
#endif

   if (m_schedInterface == NULL)
      throw std::runtime_error("unable to create a hardware scheduler interface");

   BVK_RegisterMemoryInterface(m_memoryInterface);
   BVK_RegisterSchedInterface(m_schedInterface);
}

BVKPlatform::~BVKPlatform()
{
   // It is OK to clear and delete the registered interfaces after we have
   // shutdown Nexus. Doing this ensures the platform code is in a state
   // which allows it to be initialized again later.
   BVK_RegisterMemoryInterface(NULL);
   BVK_RegisterSchedInterface(NULL);

   // The destroy APIs are nullptr safe
   DestroyDRMMemoryInterface(m_memoryInterface);

#ifdef VK_USE_PLATFORM_ANDROID_KHR
   DestroyAndroidSchedInterface(m_schedInterface);
#else
   DestroySchedInterface(m_schedInterface);
#endif
}

}
#endif