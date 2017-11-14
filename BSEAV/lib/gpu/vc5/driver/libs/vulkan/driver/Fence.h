/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include <vulkan.h>

#include "ForwardDecl.h"
#include "NonCopyable.h"
#include "Allocating.h"
#include "SchedDependencies.h"

namespace bvk {

class Fence: public NonCopyable, public Allocating
{
public:
   Fence(
      const VkAllocationCallbacks   *pCallbacks,
      bvk::Device                   *pDevice,
      const VkFenceCreateInfo       *pCreateInfo);

   ~Fence() noexcept;

   VkResult GetFenceStatus(
      bvk::Device *device) noexcept;

   // Implementation specific from this point on
   bool ResetFence(
      bvk::Device *device) noexcept;

   // This replaces the internal fence with one that will
   // fire when all dependencies in deps have completed
   void ScheduleSignal(const SchedDependencies &deps) noexcept;

   void Signal() noexcept;

   // Note: static. The Device object calls this to
   // keep the code local to Fence
   static VkResult WaitForFences(
      uint32_t        fenceCount,
      const VkFence  *pFences,
      VkBool32        waitAll,
      uint64_t        timeout) noexcept;

private:
   VkResult Wait(uint64_t timeout) noexcept;

   bool IsSignalled() noexcept;

private:
   SchedDependencies m_deps;
   bool              m_isSignalled = false;
   bool              m_isQueued    = false;
};

} // namespace bvk
