/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "AllObjects.h"
#include "Common.h"

namespace bvk {

Framebuffer::Framebuffer(
   const VkAllocationCallbacks   *pCallbacks,
   bvk::Device                   *pDevice,
   const VkFramebufferCreateInfo *pCreateInfo) :
      Allocating(pCallbacks),
      m_attachments(pCreateInfo->pAttachments,
         pCreateInfo->pAttachments + pCreateInfo->attachmentCount,
         GetObjScopeAllocator<VkImageView>())
{
   m_width  = pCreateInfo->width;
   m_height = pCreateInfo->height;
   m_layers = pCreateInfo->layers;

   assert(m_width > 0 && m_height > 0 && m_layers > 0);
}

Framebuffer::~Framebuffer() noexcept
{
}

void Framebuffer::Dimensions(VkExtent3D *dims) const
{
   dims->width  = m_width;
   dims->height = m_height;
   dims->depth  = m_layers;
}

} // namespace bvk
