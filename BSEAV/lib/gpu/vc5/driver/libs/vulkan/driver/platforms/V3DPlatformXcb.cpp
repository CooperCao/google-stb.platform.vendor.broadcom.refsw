/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#ifdef VK_USE_PLATFORM_XCB_KHR

#include "Common.h"
#include "V3DPlatformBase.h"
#include "Device.h"
#include "PhysicalDevice.h"
#include "DeviceMemory.h"
#include "Image.h"
#include "Fence.h"
#include "Semaphore.h"
#include "SwapchainKHR.h"
#include "SchedDependencies.h"

#include "libs/platform/v3d_scheduler.h"
#include "libs/platform/v3d_parallel.h"
#include "libs/util/profile/profile.h"
#include "libs/platform/v3d_imgconv.h"

namespace bvk
{

#if defined(VK_USE_PLATFORM_DISPLAY_KHR)

SurfaceXcb::SurfaceXcb(const VkAllocationCallbacks *pAllocator, const VkDisplaySurfaceCreateInfoKHR *pCreateInfo) :
   Allocating(pAllocator)
{
   directDisplay = true;

   // Make a virtual direct-display using an Xcb window
   const xcb_setup_t       *setup;
   xcb_screen_iterator_t    iter;
   int                      scr;

   connection = xcb_connect(NULL, &scr);
   if (xcb_connection_has_error(connection) > 0)
   {
      printf("xcb_connect failed\n");
      throw(std::bad_alloc());
   }

   setup = xcb_get_setup(connection);
   iter = xcb_setup_roots_iterator(setup);
   while (scr-- > 0)
      xcb_screen_next(&iter);

   screen = iter.data;

   window = xcb_generate_id(connection);

   uint32_t value_mask, value_list[32];

   value_mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
   value_list[0] = screen->black_pixel;
   value_list[1] = XCB_EVENT_MASK_KEY_RELEASE | XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_STRUCTURE_NOTIFY;

   xcb_create_window(connection, XCB_COPY_FROM_PARENT, window,
      screen->root, 0, 0, pCreateInfo->imageExtent.width, pCreateInfo->imageExtent.height, 0,
      XCB_WINDOW_CLASS_INPUT_OUTPUT, screen->root_visual, value_mask, value_list);

   xcb_map_window(connection, window);

   const uint32_t coords[] = { 100, 100 };
   xcb_configure_window(connection, window, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, coords);

   // Set the window title
   const char *title = "Virtual Direct Display Mode - closing the window may cause a crash";
   xcb_change_property(connection, XCB_PROP_MODE_REPLACE, window, XCB_ATOM_WM_NAME,
                       XCB_ATOM_STRING, 8, strlen(title), title);
   xcb_flush(connection);
}

#endif // VK_USE_PLATFORM_DISPLAY_KHR

V3DPlatformXcb::V3DPlatformXcb()
{
   CommonInitialize();
}

V3DPlatformXcb::~V3DPlatformXcb()
{
   delete[] m_rgba8888;
   CommonTerminate();
}

void V3DPlatformXcb::AttachSwapchain(
   const VkAllocationCallbacks   *pCallbacks,
   bvk::Device                   *pDevice,
   SwapchainKHR                  *chain)
{
   PhysicalDevice *physDev = pDevice->GetPhysicalDevice();

   m_numImages = chain->Images().size();
   m_curImage = 0;

   // Calculate the total size of all the images
   VkDeviceSize   totalSize = 0;
   for (uint32_t i = 0; i < chain->Images().size(); i++)
   {
      VkMemoryRequirements memReq;
      chain->Images()[i]->GetImageMemoryRequirements(pDevice, &memReq);

      memReq.size = gfx_u64round_up(memReq.size, memReq.alignment);
      totalSize += memReq.size;
   }

   // Find a suitable memory type index
   uint32_t memTypeIndex = physDev->FindSuitableMemoryTypeIndex(
                              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
                              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

   VkMemoryAllocateInfo memCI;
   memCI.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
   memCI.pNext           = nullptr;
   memCI.allocationSize  = totalSize;
   memCI.memoryTypeIndex = memTypeIndex;

   // Make a single device memory block to include all images
   //   No need to try/catch, we have nothing to clean up at this point
   DeviceMemory *devMem = createObject<DeviceMemory, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT>(
                                          pCallbacks, &g_defaultAllocCallbacks, pDevice, &memCI);

   VkDeviceSize offset = 0;
   uint32_t     width  = chain->CreateInfo().imageExtent.width;
   uint32_t     height = chain->CreateInfo().imageExtent.height;

   for (uint32_t i = 0; i < chain->Images().size(); i++)
   {
      VkMemoryRequirements memReq;
      chain->Images()[i]->GetImageMemoryRequirements(pDevice, &memReq);
      memReq.size = gfx_u64round_up(memReq.size, memReq.alignment);

      // Check all images are the same size
      assert(chain->Images()[i]->Extent().width  == width);
      assert(chain->Images()[i]->Extent().height == height);

      // Bind the memory to the image
      chain->Images()[i]->BindImageMemory(pDevice, devMem, offset);

      // Set the initial state of the buffers?
      bool clearInitialBuffers = true;
      if (clearInitialBuffers)
      {
         void *memPtr = nullptr;
         devMem->MapMemory(pDevice, offset, memReq.size, 0, &memPtr);

         memset(memPtr, (i + 1) * (255 / m_numImages), static_cast<size_t>(memReq.size));

         VkMappedMemoryRange range{};
         range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
         range.size = memReq.size;
         devMem->FlushMappedRange(range);

         devMem->UnmapMemory(pDevice);
      }

      offset += memReq.size;
   }

   // Record the device memory in the swapChain
   assert(chain->DevMemBacking().size() == 0);
   chain->DevMemBacking().push_back(devMem);
}

void V3DPlatformXcb::DetachSwapchain(
   const VkAllocationCallbacks *pCallbacks,
   Device                      *pDevice,
   SwapchainKHR                *chain)
{
   for (uint32_t i = 0; i < chain->DevMemBacking().size(); i++)
      destroyObject<VK_SYSTEM_ALLOCATION_SCOPE_OBJECT>(chain->DevMemBacking()[i], pCallbacks);

   chain->DevMemBacking().clear();
}

void V3DPlatformXcb::ConvertTo8888(
   const Image &srcImage,
   void        *dstPtr,
   size_t       dstSize)
{
   GFX_BUFFER_DESC_T desc = {0};
   desc.width                 = srcImage.Extent().width;
   desc.height                = srcImage.Extent().height;
   desc.depth                 = 1;
   desc.num_planes            = 1;
   desc.planes[0].lfmt        = GFX_LFMT_B8G8R8X8_UNORM_2D_RSO;
   desc.planes[0].pitch       = desc.width * 4;

   v3d_imgconv_ptr_tgt dst;
   v3d_imgconv_init_ptr_tgt(&dst, dstPtr, &desc, 0, 0, 0, 0, 0);

   v3d_imgconv_gmem_tgt src;
   srcImage.InitGMemTarget(&src, SchedDependencies(), 0, 0);

   v3d_imgconv_convert_to_ptr(&dst, &src, desc.width, desc.height, 1,
                           1, /*secure_context=*/false);
}

void V3DPlatformXcb::ResizeDisplayBuffer(uint32_t w, uint32_t h)
{
   uint32_t size = w * h * 4;

   if (size != m_rgba8888Size || m_rgba8888 == nullptr)
   {
      if (m_rgba8888 != nullptr)
         delete[] m_rgba8888;

      m_rgba8888Size = size;
      m_rgba8888     = new uint8_t[size];
   }
}

void V3DPlatformXcb::DisplayFrame(
   Device            *device,
   SwapchainKHR      *chain,
   const Image       &image,
   const ImageData   &data)
{
   SurfaceXcb *surf = FromSurfaceKHR<SurfaceXcb>(chain->CreateInfo().surface);

   MaybeDumpPresentedImage(image);

   // We convert the src image into RGBA8888 for display
   ResizeDisplayBuffer(image.Extent().width, image.Extent().height);
   ConvertTo8888(image, m_rgba8888, m_rgba8888Size);

   // Copy the image into the window now

   // We create and free the gc and pixmap every time here as there's no
   // suitable place to free them up if we just create them once.
   // This is a test platform so the performance hit is not an issue.
   uint32_t value_mask = XCB_GC_FOREGROUND | XCB_GC_GRAPHICS_EXPOSURES;
   uint32_t value_list[2];
   value_list[0] = 0xFF0000FF;
   value_list[1] = 0;

   xcb_gcontext_t gcontext = xcb_generate_id(surf->connection);
   xcb_create_gc(surf->connection, gcontext, surf->window, value_mask, value_list);

   const xcb_setup_t *setup = xcb_get_setup(surf->connection);
   xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);
   xcb_screen_t *screen = iter.data;

   uint8_t  depth = screen->root_depth;
   uint16_t width = chain->CreateInfo().imageExtent.width;
   uint16_t height = chain->CreateInfo().imageExtent.height;

   xcb_pixmap_t pixmapId = xcb_generate_id(surf->connection);
   xcb_create_pixmap(surf->connection, depth, pixmapId, surf->window,
                     width, height);

   xcb_put_image(surf->connection, XCB_IMAGE_FORMAT_Z_PIXMAP, pixmapId, gcontext,
                width, height, 0, 0, 0, depth, m_rgba8888Size, m_rgba8888);

   xcb_copy_area(surf->connection, pixmapId, surf->window, gcontext, 0, 0, 0, 0,
                 width, height);

   xcb_flush(surf->connection);

   // Tidy up
   xcb_free_pixmap(surf->connection, pixmapId);
   xcb_free_gc(surf->connection, gcontext);
}

VkResult V3DPlatformXcb::QueueFrame(
   Device                 *device,
   const VkPresentInfoKHR *presentInfo)
{
   // Wait for the waitSemaphores to fire
   for (uint32_t s = 0; s < presentInfo->waitSemaphoreCount; s++)
   {
      Semaphore *sem = fromHandle<Semaphore>(presentInfo->pWaitSemaphores[s]);
      sem->WaitNow();
   }

   for (uint32_t c = 0; c < presentInfo->swapchainCount; c++)
   {
      SwapchainKHR *chain = fromHandle<SwapchainKHR>(presentInfo->pSwapchains[c]);
      uint32_t      indx = presentInfo->pImageIndices[c];

      DisplayFrame(device, chain, *chain->Images()[indx], chain->PlatformImageData()[indx]);
   }

   return VK_SUCCESS;
}

VkResult V3DPlatformXcb::DequeueFrame(
   SwapchainKHR *chain,
   uint64_t      timeout,
   Semaphore    *semaphore,
   Fence        *fence,
   uint32_t     *pImageIndex)
{
   // The XCB platform never has frames 'in-flight'
   // so we can signal semaphores and fences immediately
   *pImageIndex = m_curImage;
   m_curImage = (m_curImage + 1) % m_numImages;

   if (fence != nullptr)
      fence->Signal();

   if (semaphore != nullptr)
      semaphore->SignalNow();

   return VK_SUCCESS;
}

void V3DPlatformXcb::GetSurfaceExtent(
   VkExtent2D  *extent,
   VkSurfaceKHR surface) const
{
   SurfaceXcb *surf = FromSurfaceKHR<SurfaceXcb>(surface);

   xcb_get_geometry_cookie_t  geomCookie = xcb_get_geometry(surf->connection, surf->window);
   xcb_get_geometry_reply_t  *geom = xcb_get_geometry_reply(surf->connection, geomCookie, NULL);

   extent->width  = geom->width;
   extent->height = geom->height;

   free(geom);
}

} // namespace bvk

#endif // VK_USE_PLATFORM_XCB_KHR
