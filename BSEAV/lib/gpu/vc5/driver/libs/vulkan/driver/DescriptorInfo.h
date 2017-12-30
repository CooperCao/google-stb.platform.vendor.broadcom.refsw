/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include "Allocating.h"

namespace bvk {

// Wraps up a tuple of (set, binding and arrayElement)
struct DescriptorInfo
{
   DescriptorInfo() = default;

   DescriptorInfo(uint32_t ds, uint32_t bp, uint32_t element) :
      descriptorSet(ds), bindingPoint(bp), element(element)
   {}

   bool operator==(const DescriptorInfo &other) const
   {
      return element == other.element &&
             bindingPoint == other.bindingPoint &&
             descriptorSet == other.descriptorSet;
   }

   bool operator!=(const DescriptorInfo &other) const
   {
      return element != other.element ||
             bindingPoint != other.bindingPoint ||
             descriptorSet != other.descriptorSet;
   }

   bool operator<(const DescriptorInfo &other) const
   {
      if (descriptorSet == other.descriptorSet)
      {
         if (bindingPoint == other.bindingPoint)
            return element < other.element;
         else
            return bindingPoint < other.bindingPoint;
      }
      else
         return descriptorSet < other.descriptorSet;
   }

   uint32_t descriptorSet = 0;
   uint32_t bindingPoint = 0;
   uint32_t element = 0;
};

using DescriptorTable = bvk::vector<DescriptorInfo>;

class DescriptorTables
{
public:
   DescriptorTable &GetUBO()       { return m_ubo;      }
   DescriptorTable &GetSSBO()      { return m_ssbo;     }
   DescriptorTable &GetSampler()   { return m_sampler;  }
   DescriptorTable &GetImage()     { return m_image;    }

   const DescriptorInfo &GetUBO(uint32_t ix)      const { return m_ubo[ix];      }
   const DescriptorInfo &GetSSBO(uint32_t ix)     const { return m_ssbo[ix];     }
   const DescriptorInfo &GetSampler(uint32_t ix)  const { return m_sampler[ix];  }
   const DescriptorInfo &GetImage(uint32_t ix)    const { return m_image[ix];    }

   const DescriptorInfo &GetBuffer(bool ssbo, uint32_t ix) const { return ssbo ? GetSSBO(ix) : GetUBO(ix); }

private:
  // Tables of unique (set, binding, element) descriptor entries:
   DescriptorTable   m_ubo;      // UBOs
   DescriptorTable   m_ssbo;     // SSBOs
   DescriptorTable   m_sampler;  // Samplers
   DescriptorTable   m_image;    // Images or Sampled images
};

} // namespace bvk
