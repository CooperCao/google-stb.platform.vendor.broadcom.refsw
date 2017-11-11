/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "AllObjects.h"
#include "Common.h"

#include "DevMemCmdBlock.h"

#include <algorithm>

namespace bvk {

CommandPool::CommandPool(
   const VkAllocationCallbacks   *pCallbacks,
   bvk::Device                   *pDevice,
   const VkCommandPoolCreateInfo *pCreateInfo) :
      Allocating(pCallbacks),
      m_createFlags(pCreateInfo->flags),
      m_queueFamilyIndex(pCreateInfo->queueFamilyIndex),
      m_sysMemPool(pCallbacks),
      m_devMemPool(pCallbacks),
      m_devDataPool(pCallbacks),
      m_devQueryPool(pCallbacks)
{
}

CommandPool::~CommandPool() noexcept
{
   // Free all contained command buffers
   for (CommandBuffer *cmdBuf : m_cmdBuffers)
      destroyObject<VK_SYSTEM_ALLOCATION_SCOPE_OBJECT>(cmdBuf, GetCallbacks());
}

VkResult CommandPool::ResetCommandPool(
   bvk::Device             *device,
   VkCommandPoolResetFlags  flags) noexcept
{
   VkCommandBufferResetFlags cbFlags = 0;
   if (flags & VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT)
      cbFlags = cbFlags | VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT;

   // Reset all command buffers to default state
   for (CommandBuffer *cmdBuf : m_cmdBuffers)
      cmdBuf->ResetCommandBuffer(cbFlags);

   // Release all blocks back to the system if requested
   if (flags & VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT)
   {
      m_sysMemPool.CleanupAllBlocks();
      m_devMemPool.CleanupAllBlocks();
      m_devDataPool.CleanupAllBlocks();
   }

   return VK_SUCCESS;
}

void CommandPool::FreeCommandBuffers(
   bvk::Device             *device,
   uint32_t                 commandBufferCount,
   const VkCommandBuffer   *pCommandBuffers) noexcept
{
   for (uint32_t i = 0; i < commandBufferCount; i++)
   {
      if (pCommandBuffers[i] != nullptr)
      {
         CommandBuffer *cmdBuf = fromHandle<CommandBuffer>(pCommandBuffers[i]);
         if (cmdBuf != nullptr)
         {
            m_cmdBuffers.erase(cmdBuf);
            destroyObject<VK_SYSTEM_ALLOCATION_SCOPE_OBJECT>(cmdBuf, GetCallbacks());
         }
      }
   }
}

void CommandPool::TrimCommandPoolKHR(
   bvk::Device                *device,
   VkCommandPoolTrimFlagsKHR   flags) noexcept
{
   m_sysMemPool.CleanupFreeBlocks();
   m_devMemPool.CleanupFreeBlocks();
   m_devDataPool.CleanupFreeBlocks();
}

void CommandPool::AllocateCommandBuffers(
   bvk::Device                         *device,
   const VkCommandBufferAllocateInfo   *pAllocateInfo,
   VkCommandBuffer                     *pCommandBuffers)
{
   // KHR_maintenance1 says that we have to destroy any allocations that have
   // already succeeded if a later one fails, and set the pointers to null.
   memset(pCommandBuffers, 0, pAllocateInfo->commandBufferCount * sizeof(VkCommandBuffer));

   try
   {
      for (uint32_t i = 0; i < pAllocateInfo->commandBufferCount; i++)
      {
         CommandBuffer *cmdBuf = createObject<CommandBuffer, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT>(
                                           GetCallbacks(), nullptr, this, device, pAllocateInfo);
         m_cmdBuffers.insert(cmdBuf);
         pCommandBuffers[i] = toHandle<VkCommandBuffer>(cmdBuf);
      }
   }
   catch (...)
   {
      // Destroy anything we made already
      FreeCommandBuffers(device, pAllocateInfo->commandBufferCount, pCommandBuffers);
      memset(pCommandBuffers, 0, pAllocateInfo->commandBufferCount * sizeof(VkCommandBuffer));
      throw;
   }
}

DevMemCmdBlock *CommandPool::AcquireDeviceCmdBlock()
{
   return m_devMemPool.AcquireBlock();
}

void CommandPool::ReleaseDeviceCmdBlock(DevMemCmdBlock *block)
{
   m_devMemPool.ReleaseBlock(block);
}

} // namespace bvk
