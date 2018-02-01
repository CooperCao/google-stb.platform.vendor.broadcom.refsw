/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

// This simple stand-alone example demonstrates how to use the Vulkan 3D API

#include <vector>
#include <array>
#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>

#include <string.h> // for memset

#include "hello_vulkan.h"

#ifndef CLIENT_MAIN
#define CLIENT_MAIN main
#endif

#ifndef NO_V3D
namespace hello_vulkan
{

class CubeUBO
{
public:
   static constexpr uint32_t OFFSET_MVP         = 0;
   static constexpr uint32_t NUM_MVP_COMPONENTS = 16;
   static constexpr uint32_t NUM_MVP_BYTES      = NUM_MVP_COMPONENTS * sizeof(float);

   void UpdateMVP(const Mat4 &mvp)
   {
      memcpy(m_mvp, mvp.Data(), NUM_MVP_BYTES);
   }

private:
   float m_mvp[NUM_MVP_COMPONENTS];
};

// clamp -- only C++17 has a std::clamp!
template <typename T>
T clamp(const T &x, const T &mn, const T &mx)
{
   return std::min(std::max(x, mn), mx);
}

static bool IsSuccess(VkResult result)
{
   switch (result)
   {
   case VK_SUCCESS     :
   case VK_NOT_READY   :
   case VK_TIMEOUT     :
   case VK_EVENT_SET   :
   case VK_EVENT_RESET :
   case VK_INCOMPLETE  :
      return true;

   default:
      return false;
   }
}

static bool HasBitsSet(uint32_t x, uint32_t requiredBits)
{
   return (x & requiredBits) == requiredBits;
}

// Checked return value for a success code
static inline VkResult Check(VkResult result)
{
#ifndef NDEBUG
   if (!IsSuccess(result))
      throw result;
#endif

   return result;
}

#define ERR_CASE(X) case X: return #X;

static const char *ErrString(VkResult err)
{
   switch (err)
   {
   ERR_CASE(VK_SUCCESS)
   ERR_CASE(VK_NOT_READY)
   ERR_CASE(VK_TIMEOUT)
   ERR_CASE(VK_EVENT_SET)
   ERR_CASE(VK_EVENT_RESET)
   ERR_CASE(VK_INCOMPLETE)
   ERR_CASE(VK_ERROR_OUT_OF_HOST_MEMORY)
   ERR_CASE(VK_ERROR_OUT_OF_DEVICE_MEMORY)
   ERR_CASE(VK_ERROR_INITIALIZATION_FAILED)
   ERR_CASE(VK_ERROR_DEVICE_LOST)
   ERR_CASE(VK_ERROR_MEMORY_MAP_FAILED)
   ERR_CASE(VK_ERROR_LAYER_NOT_PRESENT)
   ERR_CASE(VK_ERROR_EXTENSION_NOT_PRESENT)
   ERR_CASE(VK_ERROR_FEATURE_NOT_PRESENT)
   ERR_CASE(VK_ERROR_INCOMPATIBLE_DRIVER)
   ERR_CASE(VK_ERROR_TOO_MANY_OBJECTS)
   ERR_CASE(VK_ERROR_FORMAT_NOT_SUPPORTED)
   ERR_CASE(VK_ERROR_FRAGMENTED_POOL)
   default: return "Unknown Error";
   }
}

#undef ERR_CASE

///////////////////////////////////////////////////////////////////////////////////////////////////
// PPM texture utilities
///////////////////////////////////////////////////////////////////////////////////////////////////
static void ReadTextureHeader(std::ifstream &file, uint32_t *width, uint32_t *height)
{
   std::string line;
   std::getline(file, line);

   if (line != "P6")
      throw "Not a P6 PPM file";

   // Skip comments
   do
   {
      std::getline(file, line);
   } while (line.size() > 0 && line[0] == '#');

   if (line.size() == 0)
      throw "Malformed PPM file";

   std::stringstream str(line);

   str >> *width >> *height;
}

static void ReadTextureSize(const char *fileName, uint32_t *width, uint32_t *height)
{
   std::ifstream file(fileName, std::ifstream::in | std::ifstream::binary);
   if (!file)
      throw "Couldn't open PPM file";

   ReadTextureHeader(file, width, height);
}

// Reads texture - if mappedMemory is null then
static void ReadTextureData(const char *fileName, uint32_t rowPitch, uint8_t *mappedMemory)
{
   // TODO common code with ReadTextureSize
   std::ifstream file(fileName, std::ifstream::in | std::ifstream::binary);
   if (!file)
      throw "Couldn't open PPM file";

   uint32_t width;
   uint32_t height;
   ReadTextureHeader(file, &width, &height);

   std::string line;
   std::getline(file, line);
   if (line != "255")
      throw "Expected 8-bit colour values";

   std::vector<char> data(width * height * 3);

   file.read(data.data(), data.size());
   char *src = data.data();

   for (uint32_t y = 0; y < height; ++y)
   {
      uint8_t *row = mappedMemory;

      for (uint32_t x = 0; x < width; ++x)
      {
         row[0] = *src++;
         row[1] = *src++;
         row[2] = *src++;
         row[3] = 255;
         row += 4;
      }

      mappedMemory += rowPitch;
   }
}

///////////////////////////////////////////////////////////////////////////////
// INSTANCE
///////////////////////////////////////////////////////////////////////////////
Instance::Instance(const char *name, uint32_t version)
{
   // Initialize Vulkan
   VkApplicationInfo  appInfo{};
   appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
   appInfo.pApplicationName   = name;
   appInfo.applicationVersion = version;
   appInfo.apiVersion         = VK_API_VERSION_1_0;

   uint32_t extCount;
   vkEnumerateInstanceExtensionProperties(NULL, &extCount, NULL);
   std::vector<VkExtensionProperties> exts(extCount);
   vkEnumerateInstanceExtensionProperties(NULL, &extCount, exts.data());

   std::vector<const char *>  extensions
   {
      VK_KHR_SURFACE_EXTENSION_NAME,
      VK_KHR_DISPLAY_EXTENSION_NAME
   };

   VkInstanceCreateInfo instanceCreateInfo{};
   instanceCreateInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
   instanceCreateInfo.pApplicationInfo        = &appInfo;
   instanceCreateInfo.enabledExtensionCount   = extensions.size();
   instanceCreateInfo.ppEnabledExtensionNames = extensions.data();

   Check(vkCreateInstance(&instanceCreateInfo, NO_ALLOCATOR, &m_instance));
}

Instance::~Instance()
{
   if (m_instance != VK_NULL_HANDLE)
      vkDestroyInstance(m_instance, NO_ALLOCATOR);
}

///////////////////////////////////////////////////////////////////////////////
// PHYSICAL DEVICE
///////////////////////////////////////////////////////////////////////////////
PhysicalDevice::PhysicalDevice(const Instance &instance)
{
   ////////////////////////////////////////////////////////////////////////////
   // Get the physical device
   ////////////////////////////////////////////////////////////////////////////
   uint32_t numGPU = 0;
   // How many physical devices are there?
   Check(vkEnumeratePhysicalDevices(instance, &numGPU, nullptr));

   if (numGPU == 0)
      throw "No Vulkan devices available";

   if (numGPU > 1)
      throw "Only expected one physical device on this platform";

   std::vector<VkPhysicalDevice> devices(numGPU);
   Check(vkEnumeratePhysicalDevices(instance, &numGPU, devices.data()));

   // We will use the first device since we are only expecting one
   m_device = devices[0];

   ////////////////////////////////////////////////////////////////////////////
   // Query information about this physical device
   ////////////////////////////////////////////////////////////////////////////
   vkGetPhysicalDeviceProperties(m_device, &m_properties);

   // Report our findings
   std::cout << "API    version : " << VK_VERSION_MAJOR(m_properties.apiVersion) << "." <<
                                       VK_VERSION_MINOR(m_properties.apiVersion) << "." <<
                                       VK_VERSION_PATCH(m_properties.apiVersion) << "\n";
   std::cout << "Driver version : " << m_properties.driverVersion << "\n";
   std::cout << "Vendor ID      : " << m_properties.vendorID      << "\n";
   std::cout << "Device ID      : " << m_properties.deviceID      << "\n";
   std::cout << "Device name    : " << m_properties.deviceName    << "\n";

   // Extensions
   uint32_t numDeviceExtensions;
   Check(vkEnumerateDeviceExtensionProperties(m_device, nullptr, &numDeviceExtensions, nullptr));

   std::vector<VkExtensionProperties>  deviceExtensions(numDeviceExtensions);
   Check(vkEnumerateDeviceExtensionProperties(m_device, nullptr, &numDeviceExtensions, deviceExtensions.data()));

   bool foundSwapchain = false;
   for (auto &extensionProperty : deviceExtensions)
   {
      if (extensionProperty.extensionName == std::string("VK_KHR_swapchain"))
      {
         foundSwapchain = true;
         break;
      }
   }

   if (!foundSwapchain)
      throw "Did not find VK_KHR_swapchain extension";

   // Queue families
   uint32_t numQueueFamilies;
   vkGetPhysicalDeviceQueueFamilyProperties(m_device, &numQueueFamilies, NULL);

   if (numQueueFamilies == 0)
      throw "No queue families found";

   m_queueFamilyProperties.resize(numQueueFamilies);
   vkGetPhysicalDeviceQueueFamilyProperties(m_device, &numQueueFamilies, m_queueFamilyProperties.data());

   // Physical device features
   vkGetPhysicalDeviceFeatures(m_device, &m_features);

   // Disable features we don't want -- robust buffer access
   // could imply a small performance penalty.
   m_features.robustBufferAccess = false;

   // Memory information and properties
   vkGetPhysicalDeviceMemoryProperties(m_device, &m_memoryProperties);
}

uint32_t PhysicalDevice::FindQueueIndex(VkSurfaceKHR surface) const
{
   const uint32_t queueFamilyCount = m_queueFamilyProperties.size();

   // Query queues to see if presentation is available
   std::vector<VkBool32>  supportsPresent(queueFamilyCount);

   for (uint32_t i = 0; i < queueFamilyCount; ++i)
      vkGetPhysicalDeviceSurfaceSupportKHR(m_device, i, surface, &supportsPresent[i]);

   // Try to find a queue family that has graphics and present
   for (uint32_t i = 0; i < queueFamilyCount; ++i)
   {
      if ((m_queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0 &&
          supportsPresent[i] == VK_TRUE)
         return i;
   }

   throw "No suitable queue family (needs a graphics and present queue)";
}

uint32_t PhysicalDevice::FindMemoryTypeIndex(uint32_t typeBits, VkMemoryPropertyFlags properties) const
{
   for (uint32_t i = 0; i < m_memoryProperties.memoryTypeCount; ++i)
      if (HasBitsSet(typeBits, 1 << i) &&
          HasBitsSet(m_memoryProperties.memoryTypes[i].propertyFlags, properties))
         return i;

   throw "Could not find suitable memory type index";
}

std::vector<VkPresentModeKHR> PhysicalDevice::GetPresentationModes(VkSurfaceKHR surface) const
{
   uint32_t numPresentModes;
   Check(vkGetPhysicalDeviceSurfacePresentModesKHR(m_device, surface, &numPresentModes, nullptr));

   std::vector<VkPresentModeKHR> presentModes(numPresentModes);

   Check(vkGetPhysicalDeviceSurfacePresentModesKHR(m_device, surface, &numPresentModes, presentModes.data()));

   return presentModes;
}


///////////////////////////////////////////////////////////////////////////////
// SHADER MODULE
///////////////////////////////////////////////////////////////////////////////
ShaderModule::ShaderModule(VkDevice device, const std::vector<uint32_t> &code) :
   m_device(device)
{
   VkShaderModuleCreateInfo moduleCreateInfo{};

   moduleCreateInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
   moduleCreateInfo.codeSize = code.size() * sizeof(uint32_t);
   moduleCreateInfo.pCode    = code.data();
   Check(vkCreateShaderModule(device, &moduleCreateInfo, NO_ALLOCATOR, &m_module));
}

static std::vector<uint32_t> LoadSPIRV(const char *fileName)
{
   std::ifstream file(fileName, std::ifstream::in | std::ifstream::binary);

   if (!file)
      throw "Couldn't open shader";

   file.seekg(0, file.end);
   int length = file.tellg();

   std::vector<uint32_t> spirv(length / sizeof(uint32_t));

   file.seekg(0, file.beg);
   file.read(reinterpret_cast<char *>(spirv.data()), length);

   return spirv;
}

ShaderModule::ShaderModule(VkDevice device, const char *fileName) :
   ShaderModule(device, LoadSPIRV(fileName))
{
}

ShaderModule::~ShaderModule()
{
   if (m_device != VK_NULL_HANDLE)
      vkDestroyShaderModule(m_device, m_module, NO_ALLOCATOR);
}

ShaderModule::ShaderModule(ShaderModule &&rhs) :
   m_device(rhs.m_device),
   m_module(rhs.m_module)
{
   rhs.m_device = VK_NULL_HANDLE;
   rhs.m_module = VK_NULL_HANDLE;
}

///////////////////////////////////////////////////////////////////////////////
// SURFACE
///////////////////////////////////////////////////////////////////////////////
Surface::Surface(const Instance &instance, const PhysicalDevice &physicalDevice) :
   m_instance(instance),
   m_physicalDevice(physicalDevice)
{
   // Query the display properties
   uint32_t   numDisplays;
   Check(vkGetPhysicalDeviceDisplayPropertiesKHR(m_physicalDevice, &numDisplays, nullptr));

   if (numDisplays == 0)
      throw "No displays found";

   VkDisplayPropertiesKHR  displayProperties;
   numDisplays = 1;
   Check(vkGetPhysicalDeviceDisplayPropertiesKHR(m_physicalDevice, &numDisplays, &displayProperties));

   VkDisplayKHR display = displayProperties.display;

   // Query display mode properties
   uint32_t   numModes;
   Check(vkGetDisplayModePropertiesKHR(m_physicalDevice, display, &numModes, nullptr));

   if (numModes == 0)
      throw "No display modes found";

   VkDisplayModePropertiesKHR modeProps;
   numModes = 1;
   Check(vkGetDisplayModePropertiesKHR(m_physicalDevice, display, &numModes, &modeProps));

   // Get the list of planes
   uint32_t numPlanes;
   Check(vkGetPhysicalDeviceDisplayPlanePropertiesKHR(m_physicalDevice, &numPlanes, nullptr));

   if (numPlanes == 0)
      throw "No display planes found";

   std::vector<VkDisplayPlanePropertiesKHR>  planeProperties(numPlanes);

   Check(vkGetPhysicalDeviceDisplayPlanePropertiesKHR(m_physicalDevice, &numPlanes, planeProperties.data()));

   // Find a compatible plane index
   uint32_t planeIndex = FindCompatiblePlane(display, planeProperties);

   // Query the plane capabilities
   VkDisplayPlaneCapabilitiesKHR planeCapabilies;
   vkGetDisplayPlaneCapabilitiesKHR(m_physicalDevice, modeProps.displayMode, planeIndex, &planeCapabilies);

   std::array<VkDisplayPlaneAlphaFlagBitsKHR, 4> alphaModes
   {
      VK_DISPLAY_PLANE_ALPHA_OPAQUE_BIT_KHR,
      VK_DISPLAY_PLANE_ALPHA_GLOBAL_BIT_KHR,
      VK_DISPLAY_PLANE_ALPHA_PER_PIXEL_BIT_KHR,
      VK_DISPLAY_PLANE_ALPHA_PER_PIXEL_PREMULTIPLIED_BIT_KHR,
   };

   VkDisplayPlaneAlphaFlagBitsKHR alphaMode = VK_DISPLAY_PLANE_ALPHA_OPAQUE_BIT_KHR;

   for (VkDisplayPlaneAlphaFlagBitsKHR mode : alphaModes)
   {
      if (planeCapabilies.supportedAlpha & mode)
      {
         alphaMode = mode;
         break;
      }
   }

   VkExtent2D imageExtent;
   imageExtent.width  = modeProps.parameters.visibleRegion.width;
   imageExtent.height = modeProps.parameters.visibleRegion.height;

   VkDisplaySurfaceCreateInfoKHR surfaceCreateInfo{};
   surfaceCreateInfo.sType           = VK_STRUCTURE_TYPE_DISPLAY_SURFACE_CREATE_INFO_KHR;
   surfaceCreateInfo.displayMode     = modeProps.displayMode;
   surfaceCreateInfo.planeIndex      = planeIndex;
   surfaceCreateInfo.planeStackIndex = planeProperties[planeIndex].currentStackIndex;
   surfaceCreateInfo.transform       = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
   surfaceCreateInfo.alphaMode       = alphaMode;
   surfaceCreateInfo.globalAlpha     = 1.0f;
   surfaceCreateInfo.imageExtent     = imageExtent;

   Check(vkCreateDisplayPlaneSurfaceKHR(m_instance, &surfaceCreateInfo, NO_ALLOCATOR, &m_surface));

   m_format = FindFormat(m_physicalDevice, m_surface);
}

uint32_t Surface::FindCompatiblePlane(VkDisplayKHR display, const std::vector<VkDisplayPlanePropertiesKHR> &planeProperties) const
{
   // Find a plane compatible with the display
   for (uint32_t pi = 0; pi < planeProperties.size(); ++pi)
   {
      if ((planeProperties[pi].currentDisplay != VK_NULL_HANDLE) &&
          (planeProperties[pi].currentDisplay != display))
         continue;

      uint32_t numSupported;
      Check(vkGetDisplayPlaneSupportedDisplaysKHR(m_physicalDevice, pi, &numSupported, nullptr));

      if (numSupported == 0)
         continue;

      std::vector<VkDisplayKHR>  supportedDisplays(numSupported);

      Check(vkGetDisplayPlaneSupportedDisplaysKHR(m_physicalDevice, pi, &numSupported, supportedDisplays.data()));

      for (uint32_t i = 0; i < numSupported; ++i)
         if (supportedDisplays[i] == display)
            return pi;
   }

   throw "No plane found compatible with the display";
}

VkSurfaceFormatKHR Surface::FindFormat(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
   uint32_t numFormats;
   Check(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &numFormats, nullptr));

   if (numFormats == 0)
      throw "No device surface formats found";

   std::vector<VkSurfaceFormatKHR> surfaceFormats(numFormats);

   Check(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &numFormats, surfaceFormats.data()));

   VkSurfaceFormatKHR &format = surfaceFormats[0];

   if (numFormats == 1 && format.format == VK_FORMAT_UNDEFINED)
      format.format = VK_FORMAT_R8G8B8A8_UNORM;

   return surfaceFormats[0];
}

Surface::~Surface()
{
   if (m_instance != VK_NULL_HANDLE)
      vkDestroySurfaceKHR(m_instance, m_surface, NO_ALLOCATOR);
}

///////////////////////////////////////////////////////////////////////////////
// DEVICE
///////////////////////////////////////////////////////////////////////////////
Device::Device(const PhysicalDevice &physicalDevice, const Surface &surface) :
   m_physicalDevice(physicalDevice)
{
   m_queueFamilyIndex = physicalDevice.FindQueueIndex(surface);

   float queuePriorities[1] = { 0.0 };

   VkDeviceQueueCreateInfo queueCreateInfo[1]{};

   queueCreateInfo[0].sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
   queueCreateInfo[0].queueFamilyIndex = m_queueFamilyIndex;
   queueCreateInfo[0].queueCount       = 1;
   queueCreateInfo[0].pQueuePriorities = queuePriorities;

   VkDeviceCreateInfo deviceCreateInfo{};

   static const char *extensions[] =
   {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME
   };

   deviceCreateInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
   deviceCreateInfo.queueCreateInfoCount    = 1;
   deviceCreateInfo.pQueueCreateInfos       = queueCreateInfo;
   deviceCreateInfo.enabledExtensionCount   = 1;
   deviceCreateInfo.ppEnabledExtensionNames = extensions;
   deviceCreateInfo.pEnabledFeatures        = &m_physicalDevice.GetFeatures();
   deviceCreateInfo.queueCreateInfoCount    = 1;

   Check(vkCreateDevice(physicalDevice, &deviceCreateInfo, NO_ALLOCATOR, &m_device));

   vkGetDeviceQueue(m_device, m_queueFamilyIndex, 0, &m_queue);
}

Device::~Device()
{
   if (m_device != VK_NULL_HANDLE)
      vkDestroyDevice(m_device, NO_ALLOCATOR);
}

void Device::Wait() const
{
   Check(vkQueueWaitIdle(m_queue));
}

///////////////////////////////////////////////////////////////////////////////
// SYNC
///////////////////////////////////////////////////////////////////////////////

// SyncObjects
SyncObjects::SyncObjects(const Device &device) :
   m_device(device)
{
   VkFenceCreateInfo fenceCreateInfo{};
   fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
   fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

   Check(vkCreateFence(m_device, &fenceCreateInfo, NO_ALLOCATOR, &m_fence));

   VkSemaphoreCreateInfo semaphoreCreateInfo{};
   semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
   Check(vkCreateSemaphore(m_device, &semaphoreCreateInfo, NO_ALLOCATOR, &m_imageAcquiredSemaphore));
   Check(vkCreateSemaphore(m_device, &semaphoreCreateInfo, NO_ALLOCATOR, &m_drawCompleteSemaphore));
}

SyncObjects::SyncObjects(SyncObjects &&rhs) :
   m_device(rhs.m_device),
   m_fence(rhs.m_fence),
   m_imageAcquiredSemaphore(rhs.m_imageAcquiredSemaphore),
   m_drawCompleteSemaphore(rhs.m_drawCompleteSemaphore)
{
   rhs.m_device                 = VK_NULL_HANDLE;
   rhs.m_fence                  = VK_NULL_HANDLE;
   rhs.m_imageAcquiredSemaphore = VK_NULL_HANDLE;
   rhs.m_drawCompleteSemaphore  = VK_NULL_HANDLE;
}

SyncObjects::~SyncObjects()
{
   if (m_device != VK_NULL_HANDLE)
   {
      vkWaitForFences(m_device, 1, &m_fence, VK_TRUE, UINT64_MAX);
      vkDestroyFence(m_device, m_fence, NO_ALLOCATOR);
      vkDestroySemaphore(m_device, m_imageAcquiredSemaphore, NO_ALLOCATOR);
      vkDestroySemaphore(m_device, m_drawCompleteSemaphore, NO_ALLOCATOR);
   }
}

void SyncObjects::WaitFence()
{
   vkWaitForFences(m_device, 1, &m_fence, VK_TRUE, UINT64_MAX);
   vkResetFences(m_device, 1, &m_fence);
}

// Sync
Sync::Sync(const Device &device, uint32_t latency) :
   m_device(device)
{
   for (uint32_t i = 0; i < latency; ++i)
      m_syncObjects.emplace_back(device);
}

SyncObjects *Sync::Wait()
{
   SyncObjects &sync = m_syncObjects[m_frameIndex];

   sync.WaitFence();

   m_frameIndex = (m_frameIndex + 1) % m_syncObjects.size();

   return &sync;
}

///////////////////////////////////////////////////////////////////////////////
// COMMAND POOL
///////////////////////////////////////////////////////////////////////////////
CommandPool::CommandPool(const Device &device) :
   m_device(device)
{
   VkCommandPoolCreateInfo commandPoolInfo{};

   commandPoolInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
   commandPoolInfo.queueFamilyIndex = device.GetQueueFamilyIndex();

   Check(vkCreateCommandPool(m_device, &commandPoolInfo, NO_ALLOCATOR, &m_pool));
}

CommandPool::~CommandPool()
{
   if (m_device != VK_NULL_HANDLE)
      vkDestroyCommandPool(m_device, m_pool, NO_ALLOCATOR);
}

///////////////////////////////////////////////////////////////////////////////
// COMMAND BUFFER
///////////////////////////////////////////////////////////////////////////////
CommandBuffer::CommandBuffer(const Device &device, const CommandPool &commandPool, VkCommandBufferLevel level) :
   m_device(device),
   m_pool(commandPool)
{
   VkCommandBufferAllocateInfo commandBufferAllocInfo{};

   commandBufferAllocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
   commandBufferAllocInfo.commandPool        = commandPool;
   commandBufferAllocInfo.level              = level;
   commandBufferAllocInfo.commandBufferCount = 1;

   Check(vkAllocateCommandBuffers(m_device, &commandBufferAllocInfo, &m_buffer));
}

CommandBuffer::CommandBuffer(CommandBuffer &&rhs) :
   m_device(rhs.m_device),
   m_pool(rhs.m_pool),
   m_buffer(rhs.m_buffer)
{
   rhs.m_device = VK_NULL_HANDLE;
   rhs.m_pool   = VK_NULL_HANDLE;
   rhs.m_buffer = VK_NULL_HANDLE;
}

CommandBuffer::~CommandBuffer()
{
   if (m_device != VK_NULL_HANDLE)
      vkFreeCommandBuffers(m_device, m_pool, 1, &m_buffer);
}

void CommandBuffer::Begin(VkCommandBufferUsageFlags flags) const
{
   VkCommandBufferBeginInfo commandBufferBeginInfo{};
   commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
   commandBufferBeginInfo.flags = flags;

   Check(vkBeginCommandBuffer(m_buffer, &commandBufferBeginInfo));
}

void CommandBuffer::End() const
{
   Check(vkEndCommandBuffer(m_buffer));
}

void CommandBuffer::SubmitAndWait(VkQueue queue) const
{
   VkSubmitInfo submitInfo{};
   submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
   submitInfo.commandBufferCount = 1;
   submitInfo.pCommandBuffers    = &m_buffer;

   Check(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));
   Check(vkQueueWaitIdle(queue));
}

void CommandBuffer::CopyImage(
   VkImage srcImage, VkImageLayout srcImageLayout,
   VkImage dstImage, VkImageLayout dstImageLayout,
   uint32_t regionCount, const VkImageCopy *pRegions) const
{
   vkCmdCopyImage(m_buffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
}

void CommandBuffer::BlitImage(
   VkImage srcImage, VkImageLayout srcImageLayout,
   VkImage dstImage, VkImageLayout dstImageLayout,
   uint32_t regionCount, const VkImageBlit *pRegions,
   VkFilter filter) const
{
   vkCmdBlitImage(m_buffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter);
}


void CommandBuffer::PipelineBarrier(
   VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
   VkDependencyFlags dependencyFlags,
   uint32_t memoryBarrierCount,       const VkMemoryBarrier       *pMemoryBarriers,
   uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers,
   uint32_t imageMemoryBarrierCount,  const VkImageMemoryBarrier  *pImageMemoryBarriers) const
{
   vkCmdPipelineBarrier(m_buffer, srcStageMask, dstStageMask, dependencyFlags,
                        memoryBarrierCount,       pMemoryBarriers,
                        bufferMemoryBarrierCount, pBufferMemoryBarriers,
                        imageMemoryBarrierCount,  pImageMemoryBarriers);
}

void CommandBuffer::SetImageLayout(
   VkImage image, VkImageAspectFlags aspectMask,
   VkImageLayout oldLayout, VkImageLayout newLayout,
   VkAccessFlagBits srcAccessMask,
   VkPipelineStageFlags srcStages, VkPipelineStageFlags destStages) const
{
   VkImageMemoryBarrier imageMemoryBarrier{};
   imageMemoryBarrier.sType         = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
   imageMemoryBarrier.srcAccessMask = srcAccessMask;
   imageMemoryBarrier.dstAccessMask = 0;
   imageMemoryBarrier.oldLayout     = oldLayout;
   imageMemoryBarrier.newLayout     = newLayout;
   imageMemoryBarrier.image         = image;

   imageMemoryBarrier.subresourceRange.aspectMask     = aspectMask;
   imageMemoryBarrier.subresourceRange.baseMipLevel   = 0;
   imageMemoryBarrier.subresourceRange.levelCount     = 1;
   imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
   imageMemoryBarrier.subresourceRange.layerCount     = 1;

   switch (newLayout)
   {
   case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
      imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
      break;

   case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
      imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
      break;

   case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
      imageMemoryBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
      break;

   case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
      imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
      break;

   case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
      imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
      break;

   case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
      imageMemoryBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
      break;

   default:
      imageMemoryBarrier.dstAccessMask = 0;
      break;
   }

   PipelineBarrier(srcStages, destStages, 0, 0, NULL, 0, NULL, 1, &imageMemoryBarrier);
}

void CommandBuffer::SetViewport(uint32_t first, uint32_t count, const VkViewport *viewports) const
{
   vkCmdSetViewport(m_buffer, first, count, viewports);
}

void CommandBuffer::SetScissor(uint32_t first, uint32_t count, const VkRect2D *scissors) const
{
   vkCmdSetScissor(m_buffer, first, count, scissors);
}

void CommandBuffer::BindVertexBuffers(uint32_t first, uint32_t count, const VkBuffer *vertexBuffers, const VkDeviceSize *offsets) const
{
   vkCmdBindVertexBuffers(m_buffer, first, count, vertexBuffers, offsets);
}

void CommandBuffer::Draw(uint32_t count, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) const
{
   vkCmdDraw(m_buffer, count, instanceCount, firstVertex, firstInstance);
}

void CommandBuffer::EndRenderPass() const
{
   vkCmdEndRenderPass(m_buffer);
}

void CommandBuffer::BeginRenderPass(const VkRenderPassBeginInfo &info, VkSubpassContents contents) const
{
   vkCmdBeginRenderPass(m_buffer, &info, contents);
}

void CommandBuffer::BindPipeline(VkPipelineBindPoint bindPoint, VkPipeline pipeline) const
{
   vkCmdBindPipeline(m_buffer, bindPoint, pipeline);
}

void CommandBuffer::BindDescriptorSets(VkPipelineBindPoint bindPoint, VkPipelineLayout layout,
                                       uint32_t first, uint32_t count, const VkDescriptorSet *descriptorSets,
                                       uint32_t dynamicOffsetCount, uint32_t *dynamicOffsets) const
{
   vkCmdBindDescriptorSets(m_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout,
                           first, count, descriptorSets,
                           dynamicOffsetCount, dynamicOffsets);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// MAP MEMORY
///////////////////////////////////////////////////////////////////////////////////////////////////
MemoryMapper::MemoryMapper(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size,
                     VkMemoryMapFlags flags, void **mappedMemory) :
   m_memory(memory),
   m_device(device)
{
   Check(vkMapMemory(device, memory, offset, size, flags, mappedMemory));
}

MemoryMapper::~MemoryMapper()
{
   if (m_device != VK_NULL_HANDLE)
      vkUnmapMemory(m_device, m_memory);
}

void MemoryMapper::FlushRange(VkDeviceSize offset, VkDeviceSize size) const
{
   VkMappedMemoryRange range{};
   range.sType  = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
   range.memory = m_memory;
   range.offset = offset;
   range.size   = size;

   vkFlushMappedMemoryRanges(m_device, 1, &range);
}

void MemoryMapper::FlushAll() const
{
   FlushRange(0, VK_WHOLE_SIZE);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// TEXTURE INFO
///////////////////////////////////////////////////////////////////////////////////////////////////
static uint32_t CalcNumMipLevels(uint32_t w, uint32_t h)
{
   if (w == 0 || h == 0)
      throw "Texture has zero width and/or height";

   uint32_t largest = std::max(w, h) >> 1;
   uint32_t log     = 0;

   while (largest != 0)
   {
      ++log;
      largest = largest >> 1;
   }

   return log;
}

TextureInfo::TextureInfo(const char *filename) :
   m_format(VK_FORMAT_R8G8B8A8_UNORM)
{
   ReadTextureSize(filename, &m_width, &m_height);
   m_numMipLevels = CalcNumMipLevels(m_width, m_height);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// TEXTURE
///////////////////////////////////////////////////////////////////////////////////////////////////
Texture::Texture(const Device &device, const TextureInfo &info, uint32_t numMipLevels,
                 VkImageTiling tiling, VkImageUsageFlags usage, VkFlags properties) :
   m_width(info.GetWidth()),
   m_height(info.GetHeight()),
   m_numMipLevels(numMipLevels),
   m_format(info.GetFormat()),
   m_device(device)
{
   VkImageCreateInfo imageCreateInfo{};
   imageCreateInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
   imageCreateInfo.imageType     = VK_IMAGE_TYPE_2D;
   imageCreateInfo.format        = m_format;
   imageCreateInfo.extent.width  = m_width;
   imageCreateInfo.extent.height = m_height;
   imageCreateInfo.extent.depth  = 1;
   imageCreateInfo.mipLevels     = m_numMipLevels;
   imageCreateInfo.arrayLayers   = 1;
   imageCreateInfo.samples       = VK_SAMPLE_COUNT_1_BIT;
   imageCreateInfo.tiling        = tiling;
   imageCreateInfo.usage         = usage | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
   imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;

   Check(vkCreateImage(m_device, &imageCreateInfo, NO_ALLOCATOR, &m_image));

   VkMemoryRequirements memoryRequirements;
   vkGetImageMemoryRequirements(m_device, m_image, &memoryRequirements);

   VkMemoryAllocateInfo memAlloc{};
   memAlloc.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
   memAlloc.allocationSize  = memoryRequirements.size;
   memAlloc.memoryTypeIndex = device.GetPhysical().FindMemoryTypeIndex(memoryRequirements.memoryTypeBits, properties);

   Check(vkAllocateMemory(m_device, &memAlloc, NO_ALLOCATOR, &m_memory));

   Check(vkBindImageMemory(m_device, m_image, m_memory, /*offset*/0));
}

void Texture::Load(const char *fileName) const
{
   VkImageSubresource imageSubresource{};
   imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

   VkSubresourceLayout  layout;
   vkGetImageSubresourceLayout(m_device, m_image, &imageSubresource, &layout);

   {
      void       *mappedMemory;
      MemoryMapper   mapper(m_device, m_memory, /*offset*/0, VK_WHOLE_SIZE, /*flags*/0, &mappedMemory);

      ReadTextureData(fileName, layout.rowPitch, static_cast<uint8_t *>(mappedMemory));

      mapper.FlushAll();

      // Memory unmapped when mapper goes out of scope
   }
}

void Texture::TransferFrom(const CommandBuffer &commandBuffer, const Texture &from)
{
   commandBuffer.SetImageLayout(from.m_image, VK_IMAGE_ASPECT_COLOR_BIT,
                  VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_GENERAL,
                  VK_ACCESS_HOST_WRITE_BIT, VK_PIPELINE_STAGE_HOST_BIT,
                  VK_PIPELINE_STAGE_TRANSFER_BIT);

   commandBuffer.SetImageLayout(m_image, VK_IMAGE_ASPECT_COLOR_BIT,
                  VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_GENERAL,
                  (VkAccessFlagBits)0, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                  VK_PIPELINE_STAGE_TRANSFER_BIT);

   VkImageCopy imageCopy =
   {
      /*srcSubresource*/ { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 },
      /*srcOffset     */ { 0, 0, 0 },
      /*dstSubresource*/ { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 },
      /*dstOffset     */ { 0, 0, 0 },
      /*extent        */ { m_width, m_height, 1 }
   };

   // Copy the base level
   commandBuffer.CopyImage(from.m_image, VK_IMAGE_LAYOUT_GENERAL,
                           m_image,      VK_IMAGE_LAYOUT_GENERAL,
                           1, &imageCopy);

   VkImageBlit imageBlit =
   {
      /*srcSubresource*/ { /*aspectMask*/     VK_IMAGE_ASPECT_COLOR_BIT, /*mipLevel*/  0,
                           /*baseArrayLayer*/ 0,                         /*layerCount*/1 },
      /*srcOffsets    */ { { 0, 0, 0 }, { 0, 0, 1 } },
      /*dstSubresource*/ { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 },
      /*dstOffsets    */ { { 0, 0, 0 }, { 0, 0, 1 } }
   };

   uint32_t w = m_width;
   uint32_t h = m_height;

   for (uint32_t i = 1; i < GetNumMipLevels(); ++i)
   {
      imageBlit.srcSubresource.mipLevel = i - 1;
      imageBlit.srcOffsets[1].x = w;
      imageBlit.srcOffsets[1].y = h;
      w = w >> 1;
      h = h >> 1;
      imageBlit.dstSubresource.mipLevel = i;
      imageBlit.dstOffsets[1].x = w;
      imageBlit.dstOffsets[1].y = h;

      commandBuffer.SetImageLayout(m_image, VK_IMAGE_ASPECT_COLOR_BIT,
                                   VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_GENERAL,
                                   VK_ACCESS_TRANSFER_WRITE_BIT,
                                   VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_TRANSFER_READ_BIT,
                                   VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

      commandBuffer.BlitImage(m_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                              m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                              1, &imageBlit, VK_FILTER_LINEAR);
   }

   commandBuffer.SetImageLayout(m_image, VK_IMAGE_ASPECT_COLOR_BIT,
                  VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                  VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                  VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
}

Texture::~Texture()
{
   if (m_device != VK_NULL_HANDLE)
   {
      vkFreeMemory(m_device, m_memory, NO_ALLOCATOR);
      vkDestroyImage(m_device, m_image, NO_ALLOCATOR);
   }
}

///////////////////////////////////////////////////////////////////////////////
// SAMPLER
///////////////////////////////////////////////////////////////////////////////
Sampler::Sampler(const Device &device, uint32_t numMipLevels) :
   m_device(device)
{
   VkSamplerCreateInfo samplerCreateInfo{};
   samplerCreateInfo.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
   samplerCreateInfo.magFilter               = VK_FILTER_LINEAR;
   samplerCreateInfo.minFilter               = VK_FILTER_LINEAR;
   samplerCreateInfo.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR;
   samplerCreateInfo.addressModeU            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
   samplerCreateInfo.addressModeV            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
   samplerCreateInfo.addressModeW            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
   samplerCreateInfo.mipLodBias              = 0.0f;
   samplerCreateInfo.anisotropyEnable        = false;
   samplerCreateInfo.maxAnisotropy           = 1.0;
   samplerCreateInfo.compareOp               = VK_COMPARE_OP_NEVER;
   samplerCreateInfo.minLod                  = 0.0f;
   samplerCreateInfo.maxLod                  = static_cast<float>(numMipLevels);
   samplerCreateInfo.borderColor             = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
   samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;

   Check(vkCreateSampler(m_device, &samplerCreateInfo, NO_ALLOCATOR, &m_sampler));
}

Sampler::~Sampler()
{
   if (m_device != VK_NULL_HANDLE)
      vkDestroySampler(m_device, m_sampler, NO_ALLOCATOR);
}

///////////////////////////////////////////////////////////////////////////////
// TEXTURE VIEW
///////////////////////////////////////////////////////////////////////////////
TextureView::TextureView(const Device &device, const Texture &texture) :
   m_device(device)
{
   VkImageViewCreateInfo viewCreateInfo{};
   viewCreateInfo.sType        = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
   viewCreateInfo.image        = VK_NULL_HANDLE;
   viewCreateInfo.viewType     = VK_IMAGE_VIEW_TYPE_2D;
   viewCreateInfo.format       = texture.GetFormat();
   viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R;
   viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
   viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;
   viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;

   viewCreateInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
   viewCreateInfo.subresourceRange.baseMipLevel   = 0;
   viewCreateInfo.subresourceRange.levelCount     = texture.GetNumMipLevels();
   viewCreateInfo.subresourceRange.baseArrayLayer = 0;
   viewCreateInfo.subresourceRange.layerCount     = 1;

   viewCreateInfo.image = texture.GetImage();

   Check(vkCreateImageView(m_device, &viewCreateInfo, NO_ALLOCATOR, &m_view));
}

TextureView::~TextureView()
{
   if (m_device != VK_NULL_HANDLE)
      vkDestroyImageView(m_device, m_view, NO_ALLOCATOR);
}

///////////////////////////////////////////////////////////////////////////////
// FRAMEBUFFER VIEW
///////////////////////////////////////////////////////////////////////////////
FramebufferView::FramebufferView(const Device &device, VkFormat format, VkImage image) :
   m_device(device)
{
   VkImageViewCreateInfo imageViewCreateInfo{};
   imageViewCreateInfo.sType        = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
   imageViewCreateInfo.format       = format;
   imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R;
   imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
   imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;
   imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;

   imageViewCreateInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
   imageViewCreateInfo.subresourceRange.baseMipLevel   = 0;
   imageViewCreateInfo.subresourceRange.levelCount     = 1;
   imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
   imageViewCreateInfo.subresourceRange.layerCount     = 1;

   imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
   imageViewCreateInfo.image    = image;

   Check(vkCreateImageView(m_device, &imageViewCreateInfo, NO_ALLOCATOR, &m_view));
}

FramebufferView::FramebufferView(FramebufferView &&rhs) :
   m_view(rhs.m_view),
   m_device(rhs.m_device)
{
   rhs.m_view   = VK_NULL_HANDLE;
   rhs.m_device = VK_NULL_HANDLE;
}

FramebufferView::~FramebufferView()
{
   if (m_device != VK_NULL_HANDLE)
      vkDestroyImageView(m_device, m_view, NO_ALLOCATOR);
}

///////////////////////////////////////////////////////////////////////////////
// CUBE BUFFER
///////////////////////////////////////////////////////////////////////////////
const CubeVertex CubeBuffer::s_cubeVertices[CubeBuffer::NUM_VERTICES] =
{
   // X+
   { { 1.0f,  1.0f, -1.0f, }, { 1.0f, 0.0f } },
   { { 1.0f,  1.0f,  1.0f, }, { 0.0f, 0.0f } },
   { { 1.0f, -1.0f,  1.0f, }, { 0.0f, 1.0f } },
   { { 1.0f, -1.0f,  1.0f, }, { 0.0f, 1.0f } },
   { { 1.0f, -1.0f, -1.0f, }, { 1.0f, 1.0f } },
   { { 1.0f,  1.0f, -1.0f, }, { 1.0f, 0.0f } },
   // X-
   { {-1.0f, -1.0f, -1.0f, }, { 0.0f, 1.0f } },
   { {-1.0f, -1.0f,  1.0f, }, { 1.0f, 1.0f } },
   { {-1.0f,  1.0f,  1.0f, }, { 1.0f, 0.0f } },
   { {-1.0f,  1.0f,  1.0f, }, { 1.0f, 0.0f } },
   { {-1.0f,  1.0f, -1.0f, }, { 0.0f, 0.0f } },
   { {-1.0f, -1.0f, -1.0f, }, { 0.0f, 1.0f } },
   // Y+
   { {-1.0f,  1.0f, -1.0f, }, { 1.0f, 0.0f } },
   { {-1.0f,  1.0f,  1.0f, }, { 0.0f, 0.0f } },
   { { 1.0f,  1.0f,  1.0f, }, { 0.0f, 1.0f } },
   { {-1.0f,  1.0f, -1.0f, }, { 1.0f, 0.0f } },
   { { 1.0f,  1.0f,  1.0f, }, { 0.0f, 1.0f } },
   { { 1.0f,  1.0f, -1.0f, }, { 1.0f, 1.0f } },
   // Y-
   { {-1.0f, -1.0f, -1.0f, }, { 1.0f, 0.0f } },
   { { 1.0f, -1.0f, -1.0f, }, { 1.0f, 1.0f } },
   { { 1.0f, -1.0f,  1.0f, }, { 0.0f, 1.0f } },
   { {-1.0f, -1.0f, -1.0f, }, { 1.0f, 0.0f } },
   { { 1.0f, -1.0f,  1.0f, }, { 0.0f, 1.0f } },
   { {-1.0f, -1.0f,  1.0f, }, { 0.0f, 0.0f } },
   // Z+
   { {-1.0f,  1.0f,  1.0f, }, { 0.0f, 0.0f } },
   { {-1.0f, -1.0f,  1.0f, }, { 0.0f, 1.0f } },
   { { 1.0f,  1.0f,  1.0f, }, { 1.0f, 0.0f } },
   { {-1.0f, -1.0f,  1.0f, }, { 0.0f, 1.0f } },
   { { 1.0f, -1.0f,  1.0f, }, { 1.0f, 1.0f } },
   { { 1.0f,  1.0f,  1.0f, }, { 1.0f, 0.0f } },
   // Z-
   { {-1.0f, -1.0f, -1.0f, }, { 1.0f, 1.0f } },
   { { 1.0f,  1.0f, -1.0f, }, { 0.0f, 0.0f } },
   { { 1.0f, -1.0f, -1.0f, }, { 0.0f, 1.0f } },
   { {-1.0f, -1.0f, -1.0f, }, { 1.0f, 1.0f } },
   { {-1.0f,  1.0f, -1.0f, }, { 1.0f, 0.0f } },
   { { 1.0f,  1.0f, -1.0f, }, { 0.0f, 0.0f } },
};

CubeBuffer::CubeBuffer(const Device &device) :
   m_device(device)
{
   VkBufferCreateInfo bufferCreateInfo{};
   bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
   bufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
   bufferCreateInfo.size  = sizeof(CubeVertex) * NUM_VERTICES;

   Check(vkCreateBuffer(m_device, &bufferCreateInfo, NO_ALLOCATOR, &m_buffer));

   VkMemoryRequirements memoryRequirements;
   vkGetBufferMemoryRequirements(m_device, m_buffer, &memoryRequirements);

   VkMemoryAllocateInfo memoryAllocateInfo{};
   memoryAllocateInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
   memoryAllocateInfo.allocationSize  = memoryRequirements.size;
   memoryAllocateInfo.memoryTypeIndex =
      device.GetPhysical().FindMemoryTypeIndex(memoryRequirements.memoryTypeBits,
                                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                               VK_MEMORY_PROPERTY_HOST_CACHED_BIT);

   Check(vkAllocateMemory(m_device, &memoryAllocateInfo, NO_ALLOCATOR, &m_memory));

   {
      void       *vertexAddress;
      MemoryMapper   mapper(m_device, m_memory, 0, VK_WHOLE_SIZE, 0, &vertexAddress);

      memcpy(vertexAddress, &s_cubeVertices, CubeBuffer::NUM_BYTES);

      mapper.FlushAll();
   }

   Check(vkBindBufferMemory(m_device, m_buffer, m_memory, 0));
}

CubeBuffer::~CubeBuffer()
{
   if (m_device != VK_NULL_HANDLE)
   {
      vkFreeMemory(m_device, m_memory, NO_ALLOCATOR);
      vkDestroyBuffer(m_device, m_buffer, NO_ALLOCATOR);
   }
}

///////////////////////////////////////////////////////////////////////////////
// DEPTH BUFFER
///////////////////////////////////////////////////////////////////////////////
DepthBuffer::DepthBuffer(const Device &device, uint32_t width, uint32_t height, VkFormat format) :
   m_device(device),
   m_format(format)
{
   VkImageCreateInfo imageCreateInfo{};
   imageCreateInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
   imageCreateInfo.imageType     = VK_IMAGE_TYPE_2D;
   imageCreateInfo.format        = m_format;
   imageCreateInfo.extent.width  = width;
   imageCreateInfo.extent.height = height;
   imageCreateInfo.extent.depth  = 1;
   imageCreateInfo.mipLevels     = 1;
   imageCreateInfo.arrayLayers   = 1;
   imageCreateInfo.samples       = VK_SAMPLE_COUNT_1_BIT;
   imageCreateInfo.tiling        = VK_IMAGE_TILING_OPTIMAL;
   // This is a depth buffer with lazy allocation (see below)
   imageCreateInfo.usage         = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
                                   VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;

   // Create image
   Check(vkCreateImage(m_device, &imageCreateInfo, NO_ALLOCATOR, &m_image));

   VkMemoryRequirements memoryRequirements;
   vkGetImageMemoryRequirements(m_device, m_image, &memoryRequirements);

   // We request lazily allocated memory which is always available on BCM platforms
   // This means that memory will usually not be allocated to hold the depth buffer
   // as it is held on-chip and not normally written out
   VkMemoryAllocateInfo memoryAllocateInfo{};
   memoryAllocateInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
   memoryAllocateInfo.allocationSize  = memoryRequirements.size;
   memoryAllocateInfo.memoryTypeIndex =
      device.GetPhysical().FindMemoryTypeIndex(memoryRequirements.memoryTypeBits,
                                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
                                               VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT);

   Check(vkAllocateMemory(m_device, &memoryAllocateInfo, NO_ALLOCATOR, &m_memory));
   Check(vkBindImageMemory(m_device, m_image, m_memory, 0));

   VkImageViewCreateInfo viewCreateInfo{};

   viewCreateInfo.sType  = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
   viewCreateInfo.format = m_format;

   viewCreateInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_DEPTH_BIT;
   viewCreateInfo.subresourceRange.baseMipLevel   = 0;
   viewCreateInfo.subresourceRange.levelCount     = 1;
   viewCreateInfo.subresourceRange.baseArrayLayer = 0;
   viewCreateInfo.subresourceRange.layerCount     = 1;

   viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
   viewCreateInfo.image    = m_image;

   Check(vkCreateImageView(m_device, &viewCreateInfo, NO_ALLOCATOR, &m_imageView));
}

DepthBuffer::~DepthBuffer()
{
   if (m_device != VK_NULL_HANDLE)
   {
      vkDestroyImageView(m_device, m_imageView, NO_ALLOCATOR);
      vkFreeMemory(m_device, m_memory, NO_ALLOCATOR);
      vkDestroyImage(m_device, m_image, NO_ALLOCATOR);
   }
}

///////////////////////////////////////////////////////////////////////////////
// SWAPCHAIN
///////////////////////////////////////////////////////////////////////////////
Swapchain::Swapchain(const Device &device, const Surface &surface,
                     VkPresentModeKHR presentMode, uint32_t width, uint32_t height) :
   m_device(device)
{
   // Get the surface capabilities
   VkSurfaceCapabilitiesKHR surfaceCapabilities;
   Check(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.GetPhysical(), surface, &surfaceCapabilities));

   width  = clamp(width,  surfaceCapabilities.minImageExtent.width,  surfaceCapabilities.maxImageExtent.width);
   height = clamp(height, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);

   if (surfaceCapabilities.currentExtent.width != 0xFFFFFFFF)
   {
      width  = std::min(width,  surfaceCapabilities.currentExtent.width);
      height = std::min(height, surfaceCapabilities.currentExtent.height);
   }

   m_extent.width  = width;
   m_extent.height = height;

   // Get presentation modes
   std::vector<VkPresentModeKHR> presentModes = device.GetPhysical().GetPresentationModes(surface);

   // Find selected mode from:
   // VK_PRESENT_MODE_FIFO_KHR         -- no tearing, strict FIFO (support guaranteed)
   // VK_PRESENT_MODE_IMMEDIATE_KHR    -- tearing possible
   // VK_PRESENT_MODE_MAILBOX_KHR      -- no tearing, displays latest rendered frame
   // VK_PRESENT_MODE_FIFO_RELAXED_KHR -- tearing possible for late rendered images
   if (std::find(presentModes.begin(), presentModes.end(), presentMode) == presentModes.end())
      throw "Presentation mode not supported";

   // How many images do we use for the swapchain
   uint32_t numSwapchainImages = std::max(3u, surfaceCapabilities.minImageCount);

   // maxImageCount of 0 means unlimited
   if (surfaceCapabilities.maxImageCount > 0)
      numSwapchainImages = std::min(numSwapchainImages, surfaceCapabilities.maxImageCount);

   if (numSwapchainImages < 2)
      throw "Must support at least two image in the swap chain";

   VkSurfaceTransformFlagBitsKHR preTransform;
   if (surfaceCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
      preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
   else
      preTransform = surfaceCapabilities.currentTransform;

   // Composite alpha mode - at least one should be available
   VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
   VkCompositeAlphaFlagBitsKHR compositeAlphaFlags[4] =
   {
      VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
      VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
      VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
      VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
   };

   for (uint32_t i = 0; i < sizeof(compositeAlphaFlags); ++i)
   {
      if (surfaceCapabilities.supportedCompositeAlpha & compositeAlphaFlags[i])
      {
         compositeAlpha = compositeAlphaFlags[i];
         break;
      }
   }

   VkSwapchainCreateInfoKHR swapchainCreateInfo{};

   swapchainCreateInfo.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
   swapchainCreateInfo.surface          = surface;
   swapchainCreateInfo.minImageCount    = numSwapchainImages;
   swapchainCreateInfo.imageFormat      = surface.GetFormat();
   swapchainCreateInfo.imageColorSpace  = surface.GetColorSpace();
   swapchainCreateInfo.imageExtent      = m_extent;
   swapchainCreateInfo.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
   swapchainCreateInfo.preTransform     = preTransform;
   swapchainCreateInfo.compositeAlpha   = compositeAlpha;
   swapchainCreateInfo.imageArrayLayers = 1;
   swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
   swapchainCreateInfo.presentMode      = presentMode;
   swapchainCreateInfo.oldSwapchain     = VK_NULL_HANDLE;
   swapchainCreateInfo.clipped          = true;

   // Create the swapchain
   Check(vkCreateSwapchainKHR(m_device, &swapchainCreateInfo, NO_ALLOCATOR, &m_swapchain));

   // Get swapchain images
   Check(vkGetSwapchainImagesKHR(m_device, m_swapchain, &m_numImages, nullptr));

   std::vector<VkImage> swapchainImages(numSwapchainImages);
   Check(vkGetSwapchainImagesKHR(m_device, m_swapchain, &m_numImages, swapchainImages.data()));

   VkImageViewCreateInfo imageViewCreateInfo{};

   imageViewCreateInfo.sType        = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
   imageViewCreateInfo.format       = surface.GetFormat();
   imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R;
   imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
   imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;
   imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;

   imageViewCreateInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
   imageViewCreateInfo.subresourceRange.baseMipLevel   = 0;
   imageViewCreateInfo.subresourceRange.levelCount     = 1;
   imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
   imageViewCreateInfo.subresourceRange.layerCount     = 1;

   imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;

   m_image.resize(m_numImages);
   m_imageView.resize(m_numImages);

   for (uint32_t i = 0; i < m_numImages; i++)
   {
      m_image[i] = swapchainImages[i];
      imageViewCreateInfo.image = swapchainImages[i];

      Check(vkCreateImageView(m_device, &imageViewCreateInfo, NO_ALLOCATOR, &m_imageView[i]));
   }
}

Swapchain::~Swapchain()
{
   if (m_device != VK_NULL_HANDLE)
   {
      for (VkImageView imageView : m_imageView)
         vkDestroyImageView(m_device, imageView, NO_ALLOCATOR);

      vkDestroySwapchainKHR(m_device, m_swapchain, NO_ALLOCATOR);
   }
}

///////////////////////////////////////////////////////////////////////////////
// UniformBuffer
///////////////////////////////////////////////////////////////////////////////
UniformBuffer::UniformBuffer(const Device &device) :
   m_device(device)
{
   VkBufferCreateInfo bufferCreateInfo{};
   bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
   bufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
   bufferCreateInfo.size  = sizeof(CubeUBO);

   Check(vkCreateBuffer(m_device, &bufferCreateInfo, NO_ALLOCATOR, &m_buffer));

   VkMemoryRequirements memoryRequirements;
   vkGetBufferMemoryRequirements(m_device, m_buffer, &memoryRequirements);

   VkMemoryAllocateInfo memoryAllocateInfo{};
   memoryAllocateInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
   memoryAllocateInfo.allocationSize  = memoryRequirements.size;
   memoryAllocateInfo.memoryTypeIndex =
      device.GetPhysical().FindMemoryTypeIndex(memoryRequirements.memoryTypeBits,
                                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                               VK_MEMORY_PROPERTY_HOST_CACHED_BIT);

   Check(vkAllocateMemory(m_device, &memoryAllocateInfo, NO_ALLOCATOR, &m_memory));

   Check(vkBindBufferMemory(m_device, m_buffer, m_memory, 0));
}

UniformBuffer::UniformBuffer(UniformBuffer &&rhs) :
   m_device(rhs.m_device),
   m_buffer(rhs.m_buffer),
   m_memory(rhs.m_memory)
{
   rhs.m_device = VK_NULL_HANDLE;
   rhs.m_buffer = VK_NULL_HANDLE;
   rhs.m_memory = VK_NULL_HANDLE;
}

UniformBuffer::~UniformBuffer()
{
   if (m_device != VK_NULL_HANDLE)
   {
      vkFreeMemory(m_device, m_memory, NO_ALLOCATOR);
      vkDestroyBuffer(m_device, m_buffer, NO_ALLOCATOR);
   }
}


///////////////////////////////////////////////////////////////////////////////
// RENDER PASS
///////////////////////////////////////////////////////////////////////////////
RenderPass::RenderPass(const Device &device, VkFormat surfaceFormat, VkFormat depthFormat) :
   m_device(device)
{
   VkAttachmentDescription attachmentDescription[2]{};

   attachmentDescription[0].format         = surfaceFormat;
   attachmentDescription[0].samples        = VK_SAMPLE_COUNT_1_BIT;
   attachmentDescription[0].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
   attachmentDescription[0].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
   attachmentDescription[0].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
   attachmentDescription[0].finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

   attachmentDescription[1].format         = depthFormat;
   attachmentDescription[1].samples        = VK_SAMPLE_COUNT_1_BIT;
   attachmentDescription[1].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
   attachmentDescription[1].storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
   attachmentDescription[1].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
   attachmentDescription[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
   attachmentDescription[1].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
   attachmentDescription[1].finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

   VkAttachmentReference colorReference{};
   colorReference.attachment = 0;
   colorReference.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

   VkAttachmentReference depthReference{};
   depthReference.attachment = 1;
   depthReference.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

   VkSubpassDescription subpass{};
   subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
   subpass.colorAttachmentCount    = 1;
   subpass.pColorAttachments       = &colorReference;
   subpass.pDepthStencilAttachment = &depthReference;

   VkRenderPassCreateInfo renderPassCreateInfo{};
   renderPassCreateInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
   renderPassCreateInfo.attachmentCount = 2;
   renderPassCreateInfo.pAttachments    = attachmentDescription;
   renderPassCreateInfo.subpassCount    = 1;
   renderPassCreateInfo.pSubpasses      = &subpass;

   Check(vkCreateRenderPass(m_device, &renderPassCreateInfo, NO_ALLOCATOR, &m_renderPass));
}

RenderPass::~RenderPass()
{
   if (m_device != VK_NULL_HANDLE)
      vkDestroyRenderPass(m_device, m_renderPass, NO_ALLOCATOR);
}

///////////////////////////////////////////////////////////////////////////////
// DESCRIPTOR SET LAYOUT
///////////////////////////////////////////////////////////////////////////////
DescriptorSetLayout::DescriptorSetLayout(const Device &device) :
   m_device(device)
{
   VkDescriptorSetLayoutBinding layoutBindings[2]{};

   layoutBindings[0].binding         = 0;
   layoutBindings[0].descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
   layoutBindings[0].descriptorCount = 1;
   layoutBindings[0].stageFlags      = VK_SHADER_STAGE_VERTEX_BIT;

   layoutBindings[1].binding         = 1;
   layoutBindings[1].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
   layoutBindings[1].descriptorCount = 1;
   layoutBindings[1].stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT;

   VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
   descriptorSetLayoutCreateInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
   descriptorSetLayoutCreateInfo.bindingCount = 2;
   descriptorSetLayoutCreateInfo.pBindings    = layoutBindings;

   Check(vkCreateDescriptorSetLayout(m_device, &descriptorSetLayoutCreateInfo, NO_ALLOCATOR, &m_layout));
}

DescriptorSetLayout::~DescriptorSetLayout()
{
   if (m_device != VK_NULL_HANDLE)
      vkDestroyDescriptorSetLayout(m_device, m_layout, NO_ALLOCATOR);
}

///////////////////////////////////////////////////////////////////////////////
// PIPELINE LAYOUT
///////////////////////////////////////////////////////////////////////////////
PipelineLayout::PipelineLayout(const Device &device, const DescriptorSetLayout &layout) :
   m_device(device)
{
   VkDescriptorSetLayout   layouts[] = { layout };
   VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
   pipelineLayoutCreateInfo.sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
   pipelineLayoutCreateInfo.setLayoutCount = 1;
   pipelineLayoutCreateInfo.pSetLayouts    = layouts;

   Check(vkCreatePipelineLayout(m_device, &pipelineLayoutCreateInfo, NO_ALLOCATOR, &m_layout));
}

PipelineLayout::~PipelineLayout()
{
   if (m_device != VK_NULL_HANDLE)
      vkDestroyPipelineLayout(m_device, m_layout, NO_ALLOCATOR);
}

///////////////////////////////////////////////////////////////////////////////
// PIPELINE
///////////////////////////////////////////////////////////////////////////////
Pipeline::Pipeline(const Device &device, const RenderPass &renderPass, const PipelineLayout &layout) :
   m_device(device)
{
   VkVertexInputBindingDescription inputBinding{};
   inputBinding.binding   = 0;
   inputBinding.stride    = sizeof(CubeVertex);
   inputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

   VkVertexInputAttributeDescription attributeDescriptions[2]{};
   attributeDescriptions[0].location = 0;
   attributeDescriptions[0].binding  = inputBinding.binding;
   attributeDescriptions[0].format   = VK_FORMAT_R32G32B32_SFLOAT;
   attributeDescriptions[0].offset   = CubeVertex::OFFSET_POS;

   attributeDescriptions[1].location = 1;
   attributeDescriptions[1].binding  = inputBinding.binding;
   attributeDescriptions[1].format   = VK_FORMAT_R32G32_SFLOAT;
   attributeDescriptions[1].offset   = CubeVertex::OFFSET_UV;

   // VERTEX INPUT
   VkPipelineVertexInputStateCreateInfo   vertexInputState{};
   vertexInputState.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
   vertexInputState.vertexBindingDescriptionCount   = 1;
   vertexInputState.pVertexBindingDescriptions      = &inputBinding;
   vertexInputState.vertexAttributeDescriptionCount = 2;
   vertexInputState.pVertexAttributeDescriptions    = attributeDescriptions;

   // INPUT ASSEMBLY (triangle list)
   VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{};
   inputAssemblyState.sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
   inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

   // RASTERIZATION
   VkPipelineRasterizationStateCreateInfo rasterizationState{};
   rasterizationState.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
   rasterizationState.polygonMode             = VK_POLYGON_MODE_FILL;
   rasterizationState.cullMode                = VK_CULL_MODE_BACK_BIT;
   rasterizationState.frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE;
   rasterizationState.depthClampEnable        = VK_FALSE;
   rasterizationState.rasterizerDiscardEnable = VK_FALSE;
   rasterizationState.depthBiasEnable         = VK_FALSE;
   rasterizationState.lineWidth               = 1.0f;

   // COLOR BLEND ATTACHMENT
   VkPipelineColorBlendAttachmentState colorBlendAttachmentState[1]{};
   colorBlendAttachmentState[0].colorWriteMask = 0xf;
   colorBlendAttachmentState[0].blendEnable    = VK_FALSE;

   // COLOR BLEND
   VkPipelineColorBlendStateCreateInfo colorBlendState{};
   colorBlendState.sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
   colorBlendState.attachmentCount = 1;
   colorBlendState.pAttachments    = colorBlendAttachmentState;

   // VIEWPORT
   VkPipelineViewportStateCreateInfo viewportState{};
   viewportState.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
   viewportState.viewportCount = 1;
   viewportState.scissorCount  = 1;

   // DEPTH STENCIL
   VkPipelineDepthStencilStateCreateInfo depthStencilState{};
   depthStencilState.sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
   depthStencilState.depthTestEnable       = VK_TRUE;
   depthStencilState.depthWriteEnable      = VK_TRUE;
   depthStencilState.depthCompareOp        = VK_COMPARE_OP_LESS_OR_EQUAL;
   depthStencilState.depthBoundsTestEnable = VK_FALSE;
   depthStencilState.back.failOp           = VK_STENCIL_OP_KEEP;
   depthStencilState.back.passOp           = VK_STENCIL_OP_KEEP;
   depthStencilState.back.compareOp        = VK_COMPARE_OP_ALWAYS;
   depthStencilState.stencilTestEnable     = VK_FALSE;
   depthStencilState.front                 = depthStencilState.back;

   // MULTISAMPLE
   VkPipelineMultisampleStateCreateInfo  multisampleState{};
   multisampleState.sType                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
   multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

   // DYNAMIC
   VkDynamicState dynamicEnables[VK_DYNAMIC_STATE_RANGE_SIZE]{};

   VkPipelineDynamicStateCreateInfo dynamicState{};
   dynamicState.sType           = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
   dynamicState.pDynamicStates  = dynamicEnables;

   dynamicEnables[dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_VIEWPORT;
   dynamicEnables[dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_SCISSOR;

   // SHADERS
   ShaderModule vertShader(m_device, "resources_hello_vulkan/simple.vert.spv");
   ShaderModule fragShader(m_device, "resources_hello_vulkan/simple.frag.spv");

   VkPipelineShaderStageCreateInfo shaderStages[2]{};
   shaderStages[0].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
   shaderStages[0].stage  = VK_SHADER_STAGE_VERTEX_BIT;
   shaderStages[0].module = vertShader;
   shaderStages[0].pName  = "main";

   shaderStages[1].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
   shaderStages[1].stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
   shaderStages[1].module = fragShader;
   shaderStages[1].pName  = "main";

   // PIPELINE CACHE
   VkPipelineCacheCreateInfo cacheCreateInfo{};
   cacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

   Check(vkCreatePipelineCache(m_device, &cacheCreateInfo, NO_ALLOCATOR, &m_cache));

   // PIPELINE
   VkGraphicsPipelineCreateInfo createInfo{};
   createInfo.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
   createInfo.layout              = layout;
   createInfo.stageCount          = 2;
   createInfo.pVertexInputState   = &vertexInputState;
   createInfo.pInputAssemblyState = &inputAssemblyState;
   createInfo.pRasterizationState = &rasterizationState;
   createInfo.pColorBlendState    = &colorBlendState;
   createInfo.pMultisampleState   = &multisampleState;
   createInfo.pViewportState      = &viewportState;
   createInfo.pDepthStencilState  = &depthStencilState;
   createInfo.pStages             = shaderStages;
   createInfo.renderPass          = renderPass;
   createInfo.pDynamicState       = &dynamicState;

   Check(vkCreateGraphicsPipelines(m_device, m_cache, 1, &createInfo, NO_ALLOCATOR, &m_pipeline));
}

Pipeline::~Pipeline()
{
   if (m_device != VK_NULL_HANDLE)
   {
      vkDestroyPipelineCache(m_device, m_cache, NO_ALLOCATOR);
      vkDestroyPipeline(m_device, m_pipeline, NO_ALLOCATOR);
   }
}

///////////////////////////////////////////////////////////////////////////////
// DESCRIPTOR POOL
///////////////////////////////////////////////////////////////////////////////
DescriptorPool::DescriptorPool(const Device &device, bool createFreeSets, uint32_t numImages) :
   m_createFree(createFreeSets),
   m_device(device)
{
   VkDescriptorPoolSize descriptorPoolSize[2]{};
   descriptorPoolSize[0].type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
   descriptorPoolSize[0].descriptorCount = numImages;
   descriptorPoolSize[1].type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
   descriptorPoolSize[1].descriptorCount = numImages;

   VkDescriptorPoolCreateInfo descriptorPoolCreateInfo{};
   descriptorPoolCreateInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
   descriptorPoolCreateInfo.flags         = createFreeSets ? VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT : 0;
   descriptorPoolCreateInfo.maxSets       = numImages;
   descriptorPoolCreateInfo.poolSizeCount = 2;
   descriptorPoolCreateInfo.pPoolSizes    = descriptorPoolSize;

   Check(vkCreateDescriptorPool(m_device, &descriptorPoolCreateInfo, NO_ALLOCATOR,
                                &m_pool));
}

DescriptorPool::~DescriptorPool()
{
   if (m_device != VK_NULL_HANDLE)
   {
      vkResetDescriptorPool(m_device, m_pool, 0);
      vkDestroyDescriptorPool(m_device, m_pool, NO_ALLOCATOR);
   }
}

///////////////////////////////////////////////////////////////////////////////
// DESCRIPTOR SET
///////////////////////////////////////////////////////////////////////////////
DescriptorSet::DescriptorSet(const Device &device, const DescriptorPool &pool, const DescriptorSetLayout &layout,
                             const Sampler &sampler, const TextureView &textureView) :
   m_device(device),
   m_pool(pool)
{
   VkDescriptorSetLayout   layouts[] = { layout };
   VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
   descriptorSetAllocateInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
   descriptorSetAllocateInfo.descriptorPool     = pool;
   descriptorSetAllocateInfo.descriptorSetCount = 1;
   descriptorSetAllocateInfo.pSetLayouts        = layouts;

   Check(vkAllocateDescriptorSets(m_device, &descriptorSetAllocateInfo, &m_set));
}

DescriptorSet::DescriptorSet(DescriptorSet &&rhs) :
   m_device(rhs.m_device),
   m_pool(rhs.m_pool),
   m_set(rhs.m_set)
{
   rhs.m_device = VK_NULL_HANDLE;
   rhs.m_set    = VK_NULL_HANDLE;
}

DescriptorSet::~DescriptorSet()
{
   if (m_device != VK_NULL_HANDLE)
   {
      // Only free if the pool has been created so as to allow it
      if (m_pool.GetCreateFree())
         vkFreeDescriptorSets(m_device, m_pool, 1, &m_set);
   }
}

///////////////////////////////////////////////////////////////////////////////
// FRAMEBUFFER
///////////////////////////////////////////////////////////////////////////////
Framebuffer::Framebuffer(const Device &device, const RenderPass &renderPass,
                         uint32_t width, uint32_t height,
                         VkImageView colour, VkImageView depth) :
   m_device(device)
{
   VkImageView attachments[2]{};
   attachments[0] = colour;
   attachments[1] = depth;

   VkFramebufferCreateInfo framebufferCreateInfo{};
   framebufferCreateInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
   framebufferCreateInfo.renderPass      = renderPass;
   framebufferCreateInfo.attachmentCount = 2;
   framebufferCreateInfo.pAttachments    = attachments;
   framebufferCreateInfo.width           = width;
   framebufferCreateInfo.height          = height;
   framebufferCreateInfo.layers          = 1;

   Check(vkCreateFramebuffer(m_device, &framebufferCreateInfo, NO_ALLOCATOR, &m_buffer));
}

Framebuffer::Framebuffer(Framebuffer &&rhs) :
   m_device(rhs.m_device),
   m_buffer(rhs.m_buffer)
{
   rhs.m_device = VK_NULL_HANDLE;
   rhs.m_buffer = VK_NULL_HANDLE;
}

Framebuffer::~Framebuffer()
{
   if (m_device != VK_NULL_HANDLE)
      vkDestroyFramebuffer(m_device, m_buffer, NO_ALLOCATOR);
}

///////////////////////////////////////////////////////////////////////////////
// FRAME STATE
///////////////////////////////////////////////////////////////////////////////
FrameState::FrameState(const Application &app) :
   m_device(app.GetDevice()),
   m_swapchain(m_device, app.GetSurface(), app.GetOptions().GetPresentMode(),
                         app.GetOptions().GetWidth(), app.GetOptions().GetHeight()),
   m_depthBuffer(m_device, GetWidth(), GetHeight(), app.GetDepthFormat()),
   m_descriptorPool(m_device, false, m_swapchain.GetNumImages())
{
   uint32_t numImages = m_swapchain.GetNumImages();

   for (uint32_t i = 0; i < numImages; ++i)
      m_uniform.emplace_back(m_device);

   VkDescriptorBufferInfo descriptorBufferInfo{};
   descriptorBufferInfo.range = sizeof(CubeUBO);

   VkDescriptorImageInfo descriptorImageInfo{};
   descriptorImageInfo.sampler     = app.GetSampler();
   descriptorImageInfo.imageView   = app.GetTextureView();
   descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

   VkWriteDescriptorSet  writeDescriptorSet[2]{};
   writeDescriptorSet[0].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
   writeDescriptorSet[0].dstBinding      = 0;
   writeDescriptorSet[0].descriptorCount = 1;
   writeDescriptorSet[0].descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
   writeDescriptorSet[0].pBufferInfo     = &descriptorBufferInfo;

   writeDescriptorSet[1].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
   writeDescriptorSet[1].dstBinding      = 1;
   writeDescriptorSet[1].descriptorCount = 1;
   writeDescriptorSet[1].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
   writeDescriptorSet[1].pImageInfo      = &descriptorImageInfo;

   for (uint32_t i = 0; i < numImages; ++i)
   {
      m_descriptorSet.emplace_back(m_device, m_descriptorPool, app.GetDescriptorLayout(),
                                   app.GetSampler(), app.GetTextureView());

      VkDescriptorSet set = m_descriptorSet.back();

      descriptorBufferInfo.buffer  = m_uniform[i];

      writeDescriptorSet[0].dstSet = set;
      writeDescriptorSet[1].dstSet = set;

      vkUpdateDescriptorSets(m_device, 2, writeDescriptorSet, 0, NULL);
   }

   for (uint32_t i = 0; i < numImages; ++i)
      m_framebuffer.emplace_back(m_device, app.GetRenderPass(), GetWidth(), GetHeight(),
                                 m_swapchain.GetImageView(i), m_depthBuffer.GetImageView());

   for (uint32_t i = 0; i < numImages; ++i)
   {
      m_commandBuffer.emplace_back(m_device, app.GetCommandPool());
      CreateDrawCommand(app, i);
   }
}

void FrameState::CreateDrawCommand(const Application &app, uint32_t ix) const
{
   const CommandBuffer &cmd = m_commandBuffer[ix];

   cmd.Begin(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

   VkClearValue clearValues[2]{};
   clearValues[0].color.float32[0] = 0.2f;
   clearValues[0].color.float32[1] = 0.2f;
   clearValues[0].color.float32[2] = 0.2f;
   clearValues[0].color.float32[3] = 1.0f;

   clearValues[1].depthStencil.depth   = 1.0f;
   clearValues[1].depthStencil.stencil = 0;

   VkRenderPassBeginInfo renderPassBeginInfo{};
   renderPassBeginInfo.sType                    = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
   renderPassBeginInfo.renderPass               = app.GetRenderPass();
   renderPassBeginInfo.framebuffer              = m_framebuffer[ix];
   renderPassBeginInfo.renderArea.extent.width  = GetWidth();
   renderPassBeginInfo.renderArea.extent.height = GetHeight();
   renderPassBeginInfo.clearValueCount          = 2;
   renderPassBeginInfo.pClearValues             = clearValues;

   cmd.BeginRenderPass(renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
   cmd.BindPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS, app.GetPipeline());

   VkDescriptorSet descriptorSets[] = { m_descriptorSet[ix] };
   cmd.BindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, app.GetPipelineLayout(), 0, 1, descriptorSets);

   VkViewport viewport{};
   viewport.height   = static_cast<float>(GetHeight());
   viewport.width    = static_cast<float>(GetWidth());
   viewport.minDepth = 0.0f;
   viewport.maxDepth = 1.0f;

   cmd.SetViewport(0, 1, &viewport);

   VkRect2D scissor{};
   memset(&scissor, 0, sizeof(scissor));
   scissor.extent.width  = GetWidth();
   scissor.extent.height = GetHeight();
   cmd.SetScissor(0, 1, &scissor);

   VkBuffer            vertexBuffers[1] = { app.GetCubeBuffer() };
   static VkDeviceSize vertexOffsets[1] = { 0 };

   cmd.BindVertexBuffers(0, 1, vertexBuffers, vertexOffsets);
   cmd.Draw(CubeBuffer::NUM_VERTICES, 1, 0, 0);
   cmd.EndRenderPass();
   cmd.End();
}

///////////////////////////////////////////////////////////////////////////////
// APPLICATION
///////////////////////////////////////////////////////////////////////////////
static const char *TEXTURE_NAME = "resources_hello_vulkan/texture.ppm";

Application::Application(uint32_t argc, char *argv[]) :
   m_options(argc, argv),
   m_depthFormat(VK_FORMAT_D16_UNORM),
   m_instance("hello_vulkan", 1),
   m_physicalDevice(m_instance),
   m_surface(m_instance, m_physicalDevice),
   m_device(m_physicalDevice, m_surface),
   m_sync(m_device, 3),
   m_commandPool(m_device),
   m_textureInfo(TEXTURE_NAME),
   m_sampler(m_device, m_textureInfo.NumMipLevels()),
   m_cubeBuffer(m_device),
   m_renderPass(m_device, m_surface.GetFormat(), m_depthFormat),
   m_descriptorSetLayout(m_device),
   m_pipelineLayout(m_device, m_descriptorSetLayout),
   m_pipeline(m_device, m_renderPass, m_pipelineLayout),
   m_texture(m_device, m_textureInfo, m_textureInfo.NumMipLevels(), VK_IMAGE_TILING_OPTIMAL,
             VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),
   m_textureView(m_device, m_texture)
{
   LoadTexture();
}

void Application::LoadTexture()
{
   CommandBuffer  commandBuffer(m_device, m_commandPool);
   commandBuffer.Begin();

   Texture  stagingTexture(m_device, m_textureInfo, 1,
      VK_IMAGE_TILING_LINEAR, VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT);

   stagingTexture.Load(TEXTURE_NAME);

   m_texture.TransferFrom(commandBuffer, stagingTexture);

   commandBuffer.End();
   commandBuffer.SubmitAndWait(m_device.GetQueue());
   // Host texture (staging buffer) is destroyed only after the command buffer
   // is flushed and no-longer used
}

void Application::Update(uint32_t buffer)
{
   CubeUBO *ubo;
   VkDeviceMemory deviceMemory = m_frameState->GetUniform(buffer).GetMemory();

   MemoryMapper   mapper(m_device, deviceMemory, 0,
      VK_WHOLE_SIZE, 0, reinterpret_cast<void **>(&ubo));

   m_angle += 1.0f;
   Mat4 model = Rotate(m_angle, 1.0f, 1.0f, 0.0f);

   ubo->UpdateMVP(m_projection * m_view * model);

   mapper.FlushRange(CubeUBO::OFFSET_MVP, CubeUBO::NUM_MVP_BYTES);
}

void Application::UpdateAndDraw(uint32_t width, uint32_t height)
{
   SyncObjects *syncObjects = m_sync.Wait();

   // Get the index of the next available swapchain image
   // We are not expecting resize in this environment which
   // would return VK_ERROR_OUT_OF_DATE_KHR
   uint32_t buffer;
   Check(vkAcquireNextImageKHR(m_device, m_frameState->GetSwapchain(), UINT64_MAX,
      syncObjects->GetImageAcquiredSemaphore(),
      VK_NULL_HANDLE, &buffer));

   Update(buffer);

   VkPipelineStageFlags pipelineStageFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

   // Issue the draw commands
   VkSubmitInfo submitInfo{};
   submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
   submitInfo.pWaitDstStageMask = &pipelineStageFlags;
   submitInfo.waitSemaphoreCount = 1;
   submitInfo.pWaitSemaphores = &syncObjects->GetImageAcquiredSemaphore();
   submitInfo.commandBufferCount = 1;

   VkCommandBuffer commands[] = { m_frameState->GetCommandBuffer(buffer) };
   submitInfo.pCommandBuffers = commands;
   submitInfo.signalSemaphoreCount = 1;
   submitInfo.pSignalSemaphores = &syncObjects->GetDrawCompleteSemaphore();

   Check(vkQueueSubmit(m_device.GetQueue(), 1, &submitInfo, syncObjects->GetFence()));

   VkDisplayPresentInfoKHR dpi{};
   dpi.sType = VK_STRUCTURE_TYPE_DISPLAY_PRESENT_INFO_KHR;
   dpi.srcRect = { 0, 0, width, height };
   dpi.dstRect = { m_options.GetOriginX(), m_options.GetOriginY(), width, height };

   // Present the result
   VkSwapchainKHR swapchains[] = { m_frameState->GetSwapchain() };
   VkPresentInfoKHR presentInfo{};
   presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
   presentInfo.pNext = &dpi;
   presentInfo.waitSemaphoreCount = 1;
   presentInfo.pWaitSemaphores = &syncObjects->GetDrawCompleteSemaphore();
   presentInfo.swapchainCount = 1;
   presentInfo.pSwapchains = swapchains;
   presentInfo.pImageIndices = &buffer;

   Check(vkQueuePresentKHR(m_device.GetQueue(), &presentInfo));
}

void Application::Run()
{
   m_frameState = std::unique_ptr<FrameState>(new FrameState(*this));

   const Swapchain &swapchain = m_frameState->GetSwapchain();

   uint32_t width  = swapchain.GetWidth();
   uint32_t height = swapchain.GetHeight();
   float    aspect = static_cast<float>(width) / static_cast<float>(height);

   m_projection = Perspective(45.0f, aspect, 0.1f, 100.0f);
   m_view       = Translate(0.0f, 0.0f, -4.0f);

   uint64_t frame     = 0;
   uint64_t numFrames = m_options.GetFrames();

   // Run for the specified number of frames (default is 64-bit ~0 which
   // is as near to forever (10 billion years at 60fps) as makes no difference)
   while (frame < numFrames)
   {
      UpdateAndDraw(width, height);
      frame += 1;
   }

   m_device.Wait();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// OPTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////
static bool MatchPrefix(const std::string arg, const char *pfx)
{
   uint32_t len = strlen(pfx);

   return arg.substr(0, len) == pfx;
}

Options::Options(uint32_t argc, char *argv[])
{
   m_frames = ~m_frames;

   for (uint32_t i = 1; i < argc; ++i)
   {
      std::string arg(argv[i]);

      if (MatchPrefix(arg, "d="))
      {
         if (sscanf(arg.data(), "d=%dx%d", &m_width, &m_height) != 2)
            throw "Window dimensions not specified correctly";
      }
      else if (MatchPrefix(arg, "o="))
      {
         if (sscanf(arg.data(), "o=%dx%d", &m_originX, &m_originY) != 2)
            throw "Window origin not specified correctly";
      }
      else if (MatchPrefix(arg, "frames="))
      {
         uint32_t frames;
         if (sscanf(arg.data(), "frames=%d", &frames) != 1)
            throw "Number of frames not specified correctly";
         m_frames = frames;
      }
      else
      {
         throw BAD_ARGUMENT;
      }
   }
}

void Options::Usage()
{
   std::cout << "Arguments supported:\n";
   std::cout << "d=XxY          display at X by Y resolution\n";
   std::cout << "o=XxY          display at position (X,Y)\n";
   std::cout << "frames=N       run for N frames then quit\n";
}

} // namespace hello_vulkan

using namespace hello_vulkan;

int CLIENT_MAIN(int argc, char *argv[])
{
   int result = 0;

   try
   {
      Application app(argc, argv);

      app.Run();   // Run the animation loop
   }
   catch (VkResult err)
   {
      std::cerr << "Vulkan reported error '" << hello_vulkan::ErrString(err) << "'\n";
      result = 1;
   }
   catch (const char *msg)
   {
      std::cerr << "Application error '" << msg << "'\n";
      result = 2;
   }
   catch (Options::Result)
   {
      Options::Usage();
      result = 3;
   }

   return result;
}

#else
int CLIENT_MAIN(int, const char**)
{
   std::cerr << "This platform does not have V3D hardware\n";

   return 0;
}
#endif // NO_V3D
