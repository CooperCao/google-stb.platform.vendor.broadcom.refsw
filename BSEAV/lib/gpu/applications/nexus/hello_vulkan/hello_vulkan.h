/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/
#pragma once

#include <vulkan.h>
#include <cmath>
#include <memory>

#define NO_ALLOCATOR nullptr

namespace hello_vulkan
{

///////////////////////////////////////////////////////////////////////////////////////////////////
// Basic Mat4 implementation (enough for the example)
///////////////////////////////////////////////////////////////////////////////////////////////////
class Mat4
{
public:
   static constexpr uint32_t NUM_COLS     = 4;
   static constexpr uint32_t NUM_ROWS     = 4;
   static constexpr uint32_t NUM_ELEMENTS = 16;

   // Construct matrix with initializer on diagonal
   Mat4(float f = 0.0f)
   {
      uint32_t i = 0;

      for (uint32_t c = 0; c < 4; ++c)
         for (uint32_t r = 0; r < 4; ++r)
            m_elem[i++] = c == r ? f : 0.0f;
   }

   float &operator()(uint32_t i)        { return m_elem[i]; }
   float  operator()(uint32_t i) const  { return m_elem[i]; }

   float &operator()(uint32_t c, uint32_t r)       { return m_elem[c * NUM_ROWS + r]; }
   float  operator()(uint32_t c, uint32_t r) const { return m_elem[c * NUM_ROWS + r]; }

   float       *Data()       { return m_elem.data(); }
   const float *Data() const { return m_elem.data(); }

private:
   std::array<float, NUM_ELEMENTS> m_elem;
};

inline Mat4 operator*(const Mat4 &lhs, const Mat4 &rhs)
{
   Mat4 ret;

   for (uint32_t c = 0; c < 4; ++c)
   {
      for (uint32_t r = 0; r < 4; ++r)
      {
         float sum = 0.0f;

         for (uint32_t t = 0; t < 4; ++t)
            sum += lhs(t, r) * rhs(c, t);

         ret(c, r) = sum;
      }
   }

   return ret;
}

inline Mat4 Translate(float x, float y, float z)
{
   Mat4  ret(1.0f);

   ret(3, 0) = x;
   ret(3, 1) = y;
   ret(3, 2) = z;

   return ret;
}

inline Mat4 Perspective(float fovy, float aspect, float zNear, float zFar)
{
   Mat4     ret;
   float    sine, cot, deltaZ;
   float    radians = (fovy / 2.0f) * ((float)M_PI / 180.0f);

   deltaZ = zFar - zNear;
   sine = sinf(radians);
   if ((deltaZ == 0) || (sine == 0) || (aspect == 0))
      return ret;

   cot = cosf(radians) / sine;

   ret(0, 0) = cot / aspect; ret(0, 1) =    0; ret(0, 2) =                          0; ret(0, 3) =  0;
   ret(1, 0) =            0; ret(1, 1) = -cot; ret(1, 2) =                          0; ret(1, 3) =  0;
   ret(2, 0) =            0; ret(2, 1) =    0; ret(2, 2) =   -(zFar + zNear) / deltaZ; ret(2, 3) = -1;
   ret(3, 0) =            0; ret(3, 1) =    0; ret(3, 2) = -2 * zNear * zFar / deltaZ; ret(3, 3) =  0;

   return ret;
}

//! Return a rotation matrix by angle a (degrees) about axis (x, y, z)
inline Mat4 Rotate(float a, float x, float y, float z)
{
   Mat4  ret;

   float mag = sqrtf(x * x + y * y + z * z);

   if (mag > 0.0f)
   {
      float sina = sinf(a * (float)M_PI / 180.0f);
      float cosa = cosf(a * (float)M_PI / 180.0f);

      x /= mag;  y /= mag;  z /= mag;

      float xx = x * x,  yy = y * y, zz = z * z;
      float xy = x * y,  yz = y * z, zx = z * x;
      float xs = x * sina;
      float ys = y * sina;
      float zs = z * sina;
      float oneMinusCos = 1.0f - cosa;

      ret(0, 0) = (oneMinusCos * xx) + cosa;
      ret(0, 1) = (oneMinusCos * xy) + zs;
      ret(0, 2) = (oneMinusCos * zx) - ys;
      ret(0, 3) = 0.0f;

      ret(1, 0) = (oneMinusCos * xy) - zs;
      ret(1, 1) = (oneMinusCos * yy) + cosa;
      ret(1, 2) = (oneMinusCos * yz) + xs;
      ret(1, 3) = 0.0f;

      ret(2, 0) = (oneMinusCos * zx) + ys;
      ret(2, 1) = (oneMinusCos * yz) - xs;
      ret(2, 2) = (oneMinusCos * zz) + cosa;
      ret(2, 3) = 0.0f;

      ret(3, 0) = 0.0f;
      ret(3, 1) = 0.0f;
      ret(3, 2) = 0.0f;
      ret(3, 3) = 1.0f;
   }

   return ret;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// NoCopy
///////////////////////////////////////////////////////////////////////////////////////////////////
class NoCopy
{
public:
   NoCopy() {}
   NoCopy(const NoCopy &) = delete;
   NoCopy &operator=(const NoCopy &) = delete;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// SHADER MODULE - manages the lifetime of a shader module
///////////////////////////////////////////////////////////////////////////////////////////////////
class ShaderModule : public NoCopy
{
public:
   ShaderModule(VkDevice device, const std::vector<uint32_t> &code);
   ShaderModule(VkDevice device, const char *fileName);
   ShaderModule(ShaderModule &&rhs);
   ~ShaderModule();

   // Allow this to be used as a VkShaderModule
   operator VkShaderModule() const { return m_module; }

private:
   VkDevice       m_device = VK_NULL_HANDLE;
   VkShaderModule m_module = VK_NULL_HANDLE;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// MEMORY MAPPER -- manage memory mapping
///////////////////////////////////////////////////////////////////////////////////////////////////
class MemoryMapper
{
public:
   MemoryMapper(VkDevice device, VkDeviceMemory mem, VkDeviceSize offset, VkDeviceSize size,
             VkMemoryMapFlags flags, void **mappedMemory);
   ~MemoryMapper();

   void FlushRange(VkDeviceSize offset, VkDeviceSize size) const;
   void FlushAll() const;

private:
   VkDeviceMemory m_memory = VK_NULL_HANDLE;
   VkDevice       m_device = VK_NULL_HANDLE;
};

///////////////////////////////////////////////////////////////////////////////
// INSTANCE
///////////////////////////////////////////////////////////////////////////////
class Instance : public NoCopy
{
public:
   Instance(const char *name, uint32_t version);
   ~Instance();

   operator VkInstance() const { return m_instance; }

private:
   VkInstance  m_instance = VK_NULL_HANDLE;
};

///////////////////////////////////////////////////////////////////////////////
// PHYSICAL DEVICE
///////////////////////////////////////////////////////////////////////////////
class PhysicalDevice : public NoCopy
{
public:
   PhysicalDevice(const Instance &instance);

   operator VkPhysicalDevice() const { return m_device; }

   const VkPhysicalDeviceFeatures             &GetFeatures()              const { return m_features;              }
   const VkPhysicalDeviceProperties           &GetProperties()            const { return m_properties;            }
   const VkPhysicalDeviceLimits               &GetLimits()                const { return m_properties.limits;     }
   const VkPhysicalDeviceMemoryProperties     &GetMemoryProperties()      const { return m_memoryProperties;      }
   const std::vector<VkQueueFamilyProperties> &GetQueueFamilyProperties() const { return m_queueFamilyProperties; }

   std::vector<VkPresentModeKHR> GetPresentationModes(VkSurfaceKHR surface) const;
   uint32_t                      FindQueueIndex(VkSurfaceKHR surface) const;
   uint32_t                      FindMemoryTypeIndex(uint32_t typeBits, VkMemoryPropertyFlags properties) const;

private:
   VkPhysicalDeviceFeatures         m_features;
   VkPhysicalDeviceProperties       m_properties;
   VkPhysicalDeviceMemoryProperties m_memoryProperties;
   VkPhysicalDeviceLimits           m_limits;

   std::vector<VkQueueFamilyProperties> m_queueFamilyProperties;

   VkPhysicalDevice                 m_device = VK_NULL_HANDLE;
};

///////////////////////////////////////////////////////////////////////////////
// SURFACE
///////////////////////////////////////////////////////////////////////////////
class Surface : public NoCopy
{
public:
   Surface(const Instance &instance, const PhysicalDevice &physicalDevice);
   ~Surface();

   operator VkSurfaceKHR() const { return m_surface; }

   VkFormat        GetFormat()     const { return m_format.format;     }
   VkColorSpaceKHR GetColorSpace() const { return m_format.colorSpace; }

   uint32_t FindCompatiblePlane(VkDisplayKHR display, const std::vector<VkDisplayPlanePropertiesKHR> &planeProperties) const;

private:
   static VkSurfaceFormatKHR FindFormat(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

private:
   VkInstance           m_instance        = VK_NULL_HANDLE;
   VkPhysicalDevice     m_physicalDevice  = VK_NULL_HANDLE;
   VkSurfaceFormatKHR   m_format{};
   VkSurfaceKHR         m_surface         = VK_NULL_HANDLE;
};

///////////////////////////////////////////////////////////////////////////////
// DEVICE
///////////////////////////////////////////////////////////////////////////////
class Device : public NoCopy
{
public:
   Device(const PhysicalDevice &device, const Surface &surface);
   ~Device();

   operator VkDevice() const { return m_device; }

   uint32_t GetQueueFamilyIndex()      const { return m_queueFamilyIndex; }
   VkQueue  GetQueue()                 const { return m_queue;            }
   const PhysicalDevice &GetPhysical() const { return m_physicalDevice;   }

   void Wait() const;

private:
   uint32_t              m_queueFamilyIndex = UINT32_MAX;
   VkQueue               m_queue            = VK_NULL_HANDLE;
   const PhysicalDevice &m_physicalDevice;
   VkDevice              m_device           = VK_NULL_HANDLE;
};

///////////////////////////////////////////////////////////////////////////////
// SYNC -- semaphores and fences to coordinate the display
///////////////////////////////////////////////////////////////////////////////
class SyncObjects : public NoCopy
{
public:
   SyncObjects(const Device &device);
   SyncObjects(SyncObjects &&rhs);
   ~SyncObjects();

   void WaitFence();

   VkFence     &GetFence()                   { return m_fence;                   }
   VkSemaphore &GetImageAcquiredSemaphore()  { return m_imageAcquiredSemaphore;  }
   VkSemaphore &GetDrawCompleteSemaphore()   { return m_drawCompleteSemaphore;   }

private:
   VkDevice    m_device                 = VK_NULL_HANDLE;
   VkFence     m_fence                  = VK_NULL_HANDLE;
   VkSemaphore m_imageAcquiredSemaphore = VK_NULL_HANDLE;
   VkSemaphore m_drawCompleteSemaphore  = VK_NULL_HANDLE;
};

class Sync : public NoCopy
{
public:
   Sync(const Device &device, uint32_t latency);

   SyncObjects *Wait();

private:
   VkDevice                 m_device     = VK_NULL_HANDLE;
   uint32_t                 m_frameIndex = 0;
   std::vector<SyncObjects> m_syncObjects;
};

///////////////////////////////////////////////////////////////////////////////
// COMMAND POOL
///////////////////////////////////////////////////////////////////////////////
class CommandPool : public NoCopy
{
public:
   CommandPool(const Device &device);
   ~CommandPool();

   operator VkCommandPool() const { return m_pool; }

private:
   VkDevice       m_device = VK_NULL_HANDLE;
   VkCommandPool  m_pool   = VK_NULL_HANDLE;
};

///////////////////////////////////////////////////////////////////////////////
// COMMAND BUFFER
///////////////////////////////////////////////////////////////////////////////
class CommandBuffer : public NoCopy
{
public:
   CommandBuffer(const Device &device, const CommandPool &commandPool, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
   CommandBuffer(CommandBuffer &&rhs);
   ~CommandBuffer();

   void Begin(VkCommandBufferUsageFlags flags = 0) const;
   void End()   const;
   void CopyImage(VkImage srcImage, VkImageLayout srcImageLayout,
                  VkImage dstImage, VkImageLayout dstImageLayout,
                  uint32_t regionCount, const VkImageCopy *pRegions) const;
   void BlitImage(VkImage srcImage, VkImageLayout srcImageLayout,
                  VkImage dstImage, VkImageLayout dstImageLayout,
                  uint32_t regionCount, const VkImageBlit *pRegions,
                  VkFilter filter) const;
   void PipelineBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                        VkDependencyFlags dependencyFlags,
                        uint32_t memoryBarrierCount,       const VkMemoryBarrier       *pMemoryBarriers,
                        uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                        uint32_t imageMemoryBarrierCount,  const VkImageMemoryBarrier  *pImageMemoryBarriers) const;

   void SetImageLayout(VkImage image, VkImageAspectFlags aspectMask,
                       VkImageLayout oldLayout, VkImageLayout newLayout,
                       VkAccessFlagBits srcAccessMask,
                       VkPipelineStageFlags srcStages, VkPipelineStageFlags destStages) const;

   void SetViewport(uint32_t first, uint32_t count, const VkViewport *viewports) const;
   void SetScissor(uint32_t first, uint32_t count, const VkRect2D *scissors) const;
   void BindVertexBuffers(uint32_t first, uint32_t count, const VkBuffer *vertexBuffers, const VkDeviceSize *offsets) const;
   void Draw(uint32_t count, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) const;
   void EndRenderPass() const;
   void BindPipeline(VkPipelineBindPoint bindPoint, VkPipeline pipeline) const;
   void BeginRenderPass(const VkRenderPassBeginInfo &info, VkSubpassContents contents) const;
   void BindDescriptorSets(VkPipelineBindPoint bindPoint, VkPipelineLayout layout,
                           uint32_t first, uint32_t count, const VkDescriptorSet *descriptorSets,
                           uint32_t dynamicOffsetCount = 0, uint32_t *dynamicOffsets = nullptr) const;

   void SubmitAndWait(VkQueue queue) const;

   operator VkCommandBuffer() const { return m_buffer; }

private:
   VkDevice          m_device = VK_NULL_HANDLE;
   VkCommandPool     m_pool   = VK_NULL_HANDLE;
   VkCommandBuffer   m_buffer = VK_NULL_HANDLE;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// TEXTURE INFO
///////////////////////////////////////////////////////////////////////////////////////////////////
class TextureInfo
{
public:
   TextureInfo(const char *filename);

   uint32_t GetWidth()  const { return m_width;  }
   uint32_t GetHeight() const { return m_height; }
   VkFormat GetFormat() const { return m_format; }

   // Return cached number of miplevels
   uint32_t NumMipLevels() const { return m_numMipLevels; }

private:
   uint32_t    m_width;
   uint32_t    m_height;
   VkFormat    m_format;
   uint32_t    m_numMipLevels;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// TEXTURE -- resources associated with a texture
///////////////////////////////////////////////////////////////////////////////////////////////////
class Texture
{
public:
   Texture(const Device &device, const TextureInfo &info, uint32_t numMipLevels,
           VkImageTiling tiling, VkImageUsageFlags usage, VkFlags properties);
   ~Texture();

   VkFormat GetFormat() const { return m_format; }
   VkImage  GetImage()  const { return m_image;  }

   uint32_t GetWidth()        const { return m_width;        }
   uint32_t GetHeight()       const { return m_height;       }
   uint32_t GetNumMipLevels() const { return m_numMipLevels; }

   void Load(const char *fileName) const;
   void TransferFrom(const CommandBuffer &commandBuffer, const Texture &from);

private:
   uint32_t       m_width;
   uint32_t       m_height;
   uint32_t       m_numMipLevels;
   VkFormat       m_format;

   VkImage        m_image  = VK_NULL_HANDLE;
   VkDeviceMemory m_memory = VK_NULL_HANDLE;
   VkDevice       m_device = VK_NULL_HANDLE;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// SAMPLER
///////////////////////////////////////////////////////////////////////////////////////////////////
class Sampler : public NoCopy
{
public:
   Sampler(const Device &device, uint32_t mipLevels);
   ~Sampler();

   operator VkSampler() const { return m_sampler; }

private:
   VkSampler   m_sampler = VK_NULL_HANDLE;
   VkDevice    m_device  = VK_NULL_HANDLE;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// TEXTURE VIEW
///////////////////////////////////////////////////////////////////////////////////////////////////
class TextureView : public NoCopy
{
public:
   TextureView(const Device &device, const Texture &texture);
   ~TextureView();

   operator VkImageView() const { return m_view; }

private:
   VkImageView m_view   = VK_NULL_HANDLE;
   VkDevice    m_device = VK_NULL_HANDLE;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// FRAMEBUFFER VIEW
///////////////////////////////////////////////////////////////////////////////////////////////////
class FramebufferView : public NoCopy
{
public:
   FramebufferView(const Device &device, VkFormat format, VkImage image);
   FramebufferView(FramebufferView &&rhs);
   ~FramebufferView();

   operator VkImageView() const { return m_view; }

private:
   VkImageView m_view   = VK_NULL_HANDLE;
   VkDevice    m_device = VK_NULL_HANDLE;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// CUBE BUFFER -- resources associated with geometry
///////////////////////////////////////////////////////////////////////////////////////////////////
class CubeVertex
{
public:
   static constexpr uint32_t NUM_POS_COMPONENTS = 3;
   static constexpr uint32_t NUM_UV_COMPONENTS  = 2;
   static constexpr uint32_t NUM_POS_BYTES      = NUM_POS_COMPONENTS * sizeof(float);
   static constexpr uint32_t NUM_UV_BYTES       = NUM_UV_COMPONENTS  * sizeof(float);

   static constexpr uint32_t OFFSET_POS = 0;
   static constexpr uint32_t OFFSET_UV  = NUM_POS_BYTES;

   float m_position[NUM_POS_COMPONENTS];
   float m_uv[NUM_UV_COMPONENTS];
};

class CubeBuffer
{
public:
   static constexpr uint32_t   NUM_VERTICES = 36;
   static const     CubeVertex s_cubeVertices[NUM_VERTICES];
   static constexpr uint32_t   NUM_BYTES    = sizeof(s_cubeVertices);

   CubeBuffer(const Device &device);
   ~CubeBuffer();

   operator VkBuffer() const { return m_buffer; }

private:
   VkDevice          m_device = VK_NULL_HANDLE;
   VkBuffer          m_buffer = VK_NULL_HANDLE;
   VkDeviceMemory    m_memory = VK_NULL_HANDLE;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// DEPTH BUFFER
///////////////////////////////////////////////////////////////////////////////////////////////////
class DepthBuffer : public NoCopy
{
public:
   DepthBuffer(const Device &device, uint32_t width, uint32_t height, VkFormat format);
   ~DepthBuffer();

   VkFormat    GetFormat()    const { return m_format;    }
   VkImageView GetImageView() const { return m_imageView; }

private:
   VkDevice       m_device    = VK_NULL_HANDLE;
   VkFormat       m_format;
   VkImage        m_image     = VK_NULL_HANDLE;
   VkDeviceMemory m_memory    = VK_NULL_HANDLE;
   VkImageView    m_imageView = VK_NULL_HANDLE;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// SWAPCHAIN
///////////////////////////////////////////////////////////////////////////////////////////////////
class Swapchain : public NoCopy
{
public:
   Swapchain(const Device &physicalDevice, const Surface &surface,
             VkPresentModeKHR presentMode, uint32_t width, uint32_t height);

   ~Swapchain();

   operator VkSwapchainKHR() const { return m_swapchain; }

   uint32_t GetNumImages() const { return m_numImages;     }
   uint32_t GetWidth()     const { return m_extent.width;  }
   uint32_t GetHeight()    const { return m_extent.height; }

   VkImageView GetImageView(uint32_t i) const { return m_imageView[i]; }

private:
   VkDevice                 m_device    = VK_NULL_HANDLE;
   uint32_t                 m_numImages{};
   std::vector<VkImage>     m_image;
   std::vector<VkImageView> m_imageView;
   VkExtent2D               m_extent{};
   VkSwapchainKHR           m_swapchain = VK_NULL_HANDLE;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// UNIFORM BUFFER
///////////////////////////////////////////////////////////////////////////////////////////////////
class UniformBuffer : public NoCopy
{
public:
   UniformBuffer(const Device &device);
   UniformBuffer(UniformBuffer &&device);
   ~UniformBuffer();

   operator VkBuffer() const { return m_buffer; }

   VkDeviceMemory GetMemory() const { return m_memory; }

private:
   VkDevice       m_device = VK_NULL_HANDLE;
   VkBuffer       m_buffer = VK_NULL_HANDLE;
   VkDeviceMemory m_memory = VK_NULL_HANDLE;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// RENDER PASS
///////////////////////////////////////////////////////////////////////////////////////////////////
class RenderPass : public NoCopy
{
public:
   RenderPass(const Device &device, VkFormat surfaceFormat, VkFormat depthFormat);
   ~RenderPass();

   operator VkRenderPass() const { return m_renderPass; }

private:
   VkDevice       m_device     = VK_NULL_HANDLE;
   VkRenderPass   m_renderPass = VK_NULL_HANDLE;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// DESCRIPTOR SET LAYOUT
///////////////////////////////////////////////////////////////////////////////////////////////////
class DescriptorSetLayout : public NoCopy
{
public:
   DescriptorSetLayout(const Device &device);
   ~DescriptorSetLayout();

   operator VkDescriptorSetLayout() const { return m_layout; }

private:
   VkDevice                m_device = VK_NULL_HANDLE;
   VkDescriptorSetLayout   m_layout = VK_NULL_HANDLE;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// PIPELINE LAYOUT
///////////////////////////////////////////////////////////////////////////////////////////////////
class PipelineLayout : public NoCopy
{
public:
   PipelineLayout(const Device &device, const DescriptorSetLayout &layout);
   ~PipelineLayout();

   operator VkPipelineLayout() const { return m_layout; }

private:
   VkDevice          m_device = VK_NULL_HANDLE;
   VkPipelineLayout  m_layout = VK_NULL_HANDLE;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// PIPELINE
///////////////////////////////////////////////////////////////////////////////////////////////////
class Pipeline : public NoCopy
{
public:
   Pipeline(const Device &device, const RenderPass &renderPass, const PipelineLayout &layout);
   ~Pipeline();

   operator VkPipeline() const { return m_pipeline; }

private:
   VkDevice          m_device   = VK_NULL_HANDLE;
   VkPipelineCache   m_cache    = VK_NULL_HANDLE;
   VkPipeline        m_pipeline = VK_NULL_HANDLE;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// DESCRIPTOR POOL
///////////////////////////////////////////////////////////////////////////////////////////////////
class DescriptorPool : public NoCopy
{
public:
   DescriptorPool(const Device &device, bool createFreeSets, uint32_t numImages);
   ~DescriptorPool();

   operator VkDescriptorPool() const { return m_pool; }

   bool GetCreateFree() const { return m_createFree; }

private:
   bool              m_createFree = false;
   VkDevice          m_device     = VK_NULL_HANDLE;
   VkDescriptorPool  m_pool       = VK_NULL_HANDLE;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// DESCRIPTOR SET
///////////////////////////////////////////////////////////////////////////////////////////////////
class DescriptorSet : public NoCopy
{
public:
   DescriptorSet(const Device &device, const DescriptorPool &pool, const DescriptorSetLayout &layout,
                 const Sampler &sampler, const TextureView &textureView);
   DescriptorSet(DescriptorSet &&rhs);
   ~DescriptorSet();

   operator VkDescriptorSet() const { return m_set; }

private:
   VkDevice              m_device = VK_NULL_HANDLE;
   const DescriptorPool &m_pool;
   VkDescriptorSet       m_set    = VK_NULL_HANDLE;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// FRAMEBUFFER
///////////////////////////////////////////////////////////////////////////////////////////////////
class Framebuffer : public NoCopy
{
public:
   Framebuffer(const Device &device, const RenderPass &renderPass,
               uint32_t width, uint32_t height,
               VkImageView colour, VkImageView depth);
   Framebuffer(Framebuffer &&rhs);
   ~Framebuffer();

   operator VkFramebuffer() const { return m_buffer; }

private:
   VkDevice       m_device = VK_NULL_HANDLE;
   VkFramebuffer  m_buffer = VK_NULL_HANDLE;

};

class Application;

///////////////////////////////////////////////////////////////////////////////////////////////////
// FRAME STATE
///////////////////////////////////////////////////////////////////////////////////////////////////
class FrameState : public NoCopy
{
public:
   FrameState(const Application &app);

   const Swapchain     &GetSwapchain()               const { return m_swapchain;        }
   const UniformBuffer &GetUniform(uint32_t i)       const { return m_uniform[i];       }
   const CommandBuffer &GetCommandBuffer(uint32_t i) const { return m_commandBuffer[i]; }

   uint32_t GetWidth()  const { return m_swapchain.GetWidth();  }
   uint32_t GetHeight() const { return m_swapchain.GetHeight(); }

private:
   void CreateDrawCommand(const Application &app, uint32_t ix) const;

private:
   const Device              &m_device;
   Swapchain                  m_swapchain;
   DepthBuffer                m_depthBuffer;
   DescriptorPool             m_descriptorPool;
   std::vector<UniformBuffer> m_uniform;
   std::vector<DescriptorSet> m_descriptorSet;
   std::vector<Framebuffer>   m_framebuffer;
   std::vector<CommandBuffer> m_commandBuffer;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// OPTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////
class Options
{
public:
   enum Result
   {
      BAD_ARGUMENT
   };

   Options(uint32_t argc, char *argv[]);

   uint32_t         GetWidth()          const { return m_width;       }
   uint32_t         GetHeight()         const { return m_height;      }
   int32_t          GetOriginX()        const { return m_originX;     }
   int32_t          GetOriginY()        const { return m_originY;     }
   VkPresentModeKHR GetPresentMode()    const { return m_presentMode; }
   uint32_t         GetFrames()         const { return m_frames;      }

   static void      Usage();

private:
   uint32_t          m_width       = 640;
   uint32_t          m_height      = 360;
   int32_t           m_originX     = 0;
   int32_t           m_originY     = 0;
   VkPresentModeKHR  m_presentMode = VK_PRESENT_MODE_FIFO_KHR;
   uint64_t          m_frames      = 0;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// APPLICATION
///////////////////////////////////////////////////////////////////////////////////////////////////
class Application : public NoCopy
{
public:
   Application(uint32_t argc, char *argv[]);

   void Run();

   const Options     &GetOptions()     const { return m_options;      }
   VkFormat           GetDepthFormat() const { return m_depthFormat;  }

   const Device      &GetDevice()      const { return m_device;       }
   const Surface     &GetSurface()     const { return m_surface;      }
   const Sampler     &GetSampler()     const { return m_sampler;      }
   const RenderPass  &GetRenderPass()  const { return m_renderPass;   }
   const CommandPool &GetCommandPool() const { return m_commandPool;  }
   const TextureView &GetTextureView() const { return m_textureView;  }
   const Pipeline    &GetPipeline()    const { return m_pipeline;     }
   const CubeBuffer  &GetCubeBuffer()  const { return m_cubeBuffer;   }

   const PipelineLayout      &GetPipelineLayout()    const { return m_pipelineLayout;      }
   const DescriptorSetLayout &GetDescriptorLayout()  const { return m_descriptorSetLayout; }

private:
   // Helper for construction
   void LoadTexture();

   // Helpers for Run
   void UpdateAndDraw(uint32_t width, uint32_t height);
   void Update(uint32_t buffer);

private:
   // Application configuration
   Options                          m_options;
   VkFormat                         m_depthFormat;

   // Per application data (create once)
   Instance                         m_instance;
   PhysicalDevice                   m_physicalDevice;
   Surface                          m_surface;
   Device                           m_device;
   Sync                             m_sync;
   CommandPool                      m_commandPool;
   TextureInfo                      m_textureInfo;
   Sampler                          m_sampler;
   CubeBuffer                       m_cubeBuffer;
   RenderPass                       m_renderPass;
   DescriptorSetLayout              m_descriptorSetLayout;
   PipelineLayout                   m_pipelineLayout;
   Pipeline                         m_pipeline;
   Texture                          m_texture;
   TextureView                      m_textureView;

   // Potentially changes with window size (note that in BCM platform we do not get resizes)
   std::unique_ptr<FrameState>      m_frameState;

   // Transformation and animation parameters
   float                            m_angle = 0;
   Mat4                             m_view;
   Mat4                             m_projection;
};

}
