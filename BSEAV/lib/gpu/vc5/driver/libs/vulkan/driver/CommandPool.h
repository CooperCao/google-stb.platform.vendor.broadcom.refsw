/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include <vulkan.h>

#include "ForwardDecl.h"
#include "NonCopyable.h"
#include "Allocating.h"
#include "Pool.h"

namespace bvk {

class SysMemCmdBlock;
class DevMemCmdBlock;

class CommandPool: public NonCopyable, public Allocating
{
public:
   CommandPool(
      const VkAllocationCallbacks   *pCallbacks,
      bvk::Device                   *pDevice,
      const VkCommandPoolCreateInfo *pCreateInfo);

   ~CommandPool() noexcept;

   VkResult ResetCommandPool(
      bvk::Device             *device,
      VkCommandPoolResetFlags  flags) noexcept;

   void FreeCommandBuffers(
      bvk::Device             *device,
      uint32_t                 commandBufferCount,
      const VkCommandBuffer   *pCommandBuffers) noexcept;

   void TrimCommandPoolKHR(
      bvk::Device                *device,
      VkCommandPoolTrimFlagsKHR   flags) noexcept;

   // Implementation specific from this point on
   void AllocateCommandBuffers(
      bvk::Device                         *device,
      const VkCommandBufferAllocateInfo   *pAllocateInfo,
      VkCommandBuffer                     *pCommandBuffers);

   VkCommandPoolCreateFlags PoolCreateFlags() const { return m_createFlags; }

   // Called by a command buffer when it needs device memory for recording
   DevMemCmdBlock *AcquireDeviceCmdBlock();

   // Called by a command buffer to release device memory back to the pool
   void ReleaseDeviceCmdBlock(DevMemCmdBlock *block);

   // Access the system memory pool
   Pool<SysMemCmdBlock>  &SysMemPool()    { return m_sysMemPool;  }

   // Access the device data memory pool (read-only from V3D)
   Pool<DevMemDataBlock> &DevDataPool()   { return m_devDataPool; }

   // Access the device data query memory pool (read-write from V3D)
   Pool<DevMemQueryBlock> &DevQueryPool() { return m_devQueryPool; }

private:
   bvk::set<CommandBuffer*>   m_cmdBuffers;
   VkCommandPoolCreateFlags   m_createFlags;
   uint32_t                   m_queueFamilyIndex;

   Pool<SysMemCmdBlock>       m_sysMemPool;
   Pool<DevMemCmdBlock>       m_devMemPool;
   Pool<DevMemDataBlock>      m_devDataPool;
   Pool<DevMemQueryBlock>     m_devQueryPool;
};

} // namespace bvk
