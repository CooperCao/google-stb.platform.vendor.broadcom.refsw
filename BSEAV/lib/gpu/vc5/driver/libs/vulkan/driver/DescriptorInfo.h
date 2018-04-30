/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include "Allocating.h"

namespace bvk {

// Wraps up a tuple of (set, binding and arrayElement)
class DescriptorInfo
{
public:
   DescriptorInfo() = default;

   DescriptorInfo(uint32_t ds, uint32_t bp, uint32_t element) :
      m_descriptorSet(ds), m_bindingPoint(bp), m_element(element)
   {}

   DescriptorInfo(uint32_t ds, uint32_t bp, uint32_t element,
                  uint32_t offset, uint32_t stride) :
      DescriptorInfo(ds, bp, element)
   {
      m_arrayOffset = offset;
      m_arrayStride = stride;
   }

   bool operator==(const DescriptorInfo &other) const
   {
      return m_element       == other.m_element &&
             m_bindingPoint  == other.m_bindingPoint &&
             m_descriptorSet == other.m_descriptorSet;
   }

   bool operator!=(const DescriptorInfo &other) const
   {
      return m_element       != other.m_element ||
             m_bindingPoint  != other.m_bindingPoint ||
             m_descriptorSet != other.m_descriptorSet;
   }

   bool operator<(const DescriptorInfo &other) const
   {
      if (m_descriptorSet == other.m_descriptorSet)
      {
         if (m_bindingPoint == other.m_bindingPoint)
            return m_element < other.m_element;
         else
            return m_bindingPoint < other.m_bindingPoint;
      }
      else
         return m_descriptorSet < other.m_descriptorSet;
   }

   uint32_t GetDescriptorSet() const { return m_descriptorSet; }
   uint32_t GetBindingPoint()  const { return m_bindingPoint;  }
   uint32_t GetElement()       const { return m_element;       }

   uint32_t GetArrayOffset()   const { return m_arrayOffset;   }
   uint32_t GetArrayStride()   const { return m_arrayStride;    }

private:
   uint32_t m_descriptorSet = 0;
   uint32_t m_bindingPoint  = 0;
   uint32_t m_element       = 0;

   // In SSBOs this is the stride and byte offset of the trailing runtime array (if any)
   // If there is no such array offset will be ~0u and stride will be 0
   uint32_t m_arrayStride   = 0;
   uint32_t m_arrayOffset   = ~0u;
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
