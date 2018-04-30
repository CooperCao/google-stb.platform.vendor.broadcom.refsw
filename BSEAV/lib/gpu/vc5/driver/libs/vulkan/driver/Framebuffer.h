/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include <vulkan.h>

#include "ForwardDecl.h"
#include "NonCopyable.h"
#include "Allocating.h"

namespace bvk {

class Framebuffer: public NonCopyable, public Allocating
{
public:
   Framebuffer(
      const VkAllocationCallbacks   *pCallbacks,
      bvk::Device                   *pDevice,
      const VkFramebufferCreateInfo *pCreateInfo);

   ~Framebuffer() noexcept;

   // Implementation specific from this point on
   const bvk::vector<ImageView*> &Attachments() const { return m_attachments; }
   void Dimensions(VkExtent3D *dims) const;

private:
   bvk::vector<ImageView*>     m_attachments;
   uint32_t                    m_width;
   uint32_t                    m_height;
   uint32_t                    m_layers;
};

} // namespace bvk
